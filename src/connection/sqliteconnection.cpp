/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstring>

#include <QtWidgets/QApplication>

#include "property.h"
#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbresult.h"
#include "logger.h"
#include "savedfile.h"
#include "sqliteconnection.h"
#include "sqliteconnectionwidget.h"
#include "sqliteconnectioninfowidget.h"
#include "dbinterface.h"
#include "dbtableinfo.h"
#include "stringconv.h"

SQLiteConnection::SQLiteConnection(const std::string &class_id, const std::string &instance_id, DBInterface *interface)
: DBConnection (class_id, instance_id, interface), interface_(*interface), db_handle_(nullptr), prepared_command_(nullptr), prepared_command_done_(false),
  widget_(nullptr), info_widget_(nullptr)
{
    registerParameter("last_filename", &last_filename_, "");

    createSubConfigurables();
}

SQLiteConnection::~SQLiteConnection()
{
    assert (!db_handle_);

    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void SQLiteConnection::openFile (const std::string &file_name)
{
    loginf << "SQLiteConnection: openFile: " << file_name;

    last_filename_=file_name;
    assert (last_filename_.size() > 0);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    int result = sqlite3_open_v2(last_filename_.c_str(), &db_handle_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    if (result != SQLITE_OK)
    {
        // Even in case of an error we get a valid db_handle (for the
        // purpose of calling sqlite3_errmsg on it ...)
        logerr  <<  "SQLiteConnection: openFile: error " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        sqlite3_close(db_handle_);
        throw std::runtime_error ("SQLiteConnection: openFile: error");
    }
    char * sErrMsg = 0;
    sqlite3_exec(db_handle_, "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA journal_mode = OFF", NULL, NULL, &sErrMsg);

    connection_ready_ = true;

    interface_.databaseContentChanged();

    emit connectedSignal();

    if (info_widget_)
        info_widget_->updateSlot();

    QApplication::restoreOverrideCursor();
}

void SQLiteConnection::disconnect()
{
    loginf << "SQLiteConnection: disconnect";

    connection_ready_ = false;

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    if (info_widget_)
    {
        delete info_widget_;
        info_widget_ = nullptr;
    }

    if (db_handle_)
    {
        sqlite3_close(db_handle_);
        db_handle_=nullptr;
    }
}

void SQLiteConnection::executeSQL(const std::string &sql)
{
    logdbg  << "DBInterface: executeSQL: sql statement execute: '" <<sql << "'";

    char* exec_err_msg = NULL;
    int result = sqlite3_exec(db_handle_, sql.c_str(), NULL, NULL, &exec_err_msg);
    if (result != SQLITE_OK)
    {
        logerr  << "DBInterface: executeSQL: sqlite3_exec failed: " << exec_err_msg;
        sqlite3_free(exec_err_msg);
        std::string error;
        error += "DBInterface: executeSQL: sqlite3_exec failed: ";
        error += exec_err_msg;
        throw std::runtime_error (error);
    }
}

void SQLiteConnection::prepareBindStatement (const std::string &statement)
{
    const char * tail = 0;
    int ret=sqlite3_prepare_v2(db_handle_, statement.c_str(), statement.size(), &statement_, &tail);

    if (ret != SQLITE_OK)
    {
        logerr  << "DBInterface: prepareBindStatement: error preparing bind";
        return;
    }
}
void SQLiteConnection::beginBindTransaction ()
{
    char * sErrMsg = 0;
    sqlite3_exec(db_handle_, "BEGIN TRANSACTION", NULL, NULL, &sErrMsg);
}
void SQLiteConnection::stepAndClearBindings ()
{
    logdbg  << "DBInterface: stepAndClearBindings: stepping statement";
    int ret2;
    while ((ret2=sqlite3_step(statement_)))
    {
        logdbg  << "DBInterface: stepAndClearBindings: stepping returned "<< ret2;
        if (ret2 == SQLITE_DONE)
        {
            logdbg  << "DBInterface: stepAndClearBindings: bind done";
            break;
        }
        else if (ret2 > SQLITE_OK  && ret2 < SQLITE_ROW)
        {
            logerr  << "DBInterface: stepAndClearBindings: error while bind: " << ret2 << ": "
                    << sqlite3_errmsg(db_handle_);
            throw std::runtime_error ("DBInterface: stepAndClearBindings: error while bind");
        }
    }

    logdbg  << "DBInterface: stepAndClearBindings: clearing statement";

    sqlite3_clear_bindings(statement_);
    sqlite3_reset(statement_);
}

void SQLiteConnection::endBindTransaction ()
{
    char * sErrMsg = 0;
    sqlite3_exec(db_handle_, "END TRANSACTION", NULL, NULL, &sErrMsg);
}
void SQLiteConnection::finalizeBindStatement ()
{
    sqlite3_finalize(statement_);
}

void SQLiteConnection::bindVariable (unsigned int index, int value)
{
    logdbg  << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_int(statement_, index, value);
}
void SQLiteConnection::bindVariable (unsigned int index, double value)
{
    logdbg  << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_double(statement_, index, value);
}
void SQLiteConnection::bindVariable (unsigned int index, const std::string &value)
{
    logdbg  << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_text(statement_, index, value.c_str(), -1, SQLITE_TRANSIENT);
}

void SQLiteConnection::bindVariableNull (unsigned int index)
{
    sqlite3_bind_null(statement_, index);
}

// TODO: beware of se deleted propertylist, new buffer should use deep copied list
std::shared_ptr <DBResult> SQLiteConnection::execute (const DBCommand &command)
{
    std::shared_ptr <DBResult> dbresult (new DBResult ());
    std::string sql = command.get();

    if (command.resultList().size() > 0) // data should be returned
    {
        std::shared_ptr <Buffer> buffer (new Buffer (command.resultList()));
        dbresult->buffer(buffer);
        logdbg  << "MySQLppConnection: execute: executing";
        execute (sql, buffer);
    }
    else
    {
        logdbg  << "MySQLppConnection: execute: executing";
        execute (sql);
    }

    logdbg  << "SQLiteConnection: execute: end";

    return dbresult;
}

std::shared_ptr <DBResult> SQLiteConnection::execute (const DBCommandList &command_list)
{
    std::shared_ptr <DBResult> dbresult (new DBResult ());

    unsigned int num_commands = command_list.getNumCommands();

    if (command_list.getResultList().size() > 0) // data should be returned
    {
        std::shared_ptr <Buffer> buffer (new Buffer (command_list.getResultList()));
        dbresult->buffer(buffer);

        for (unsigned int cnt=0; cnt < num_commands; cnt++)
            execute (command_list.getCommandString(cnt), buffer);
    }
    else
    {
        for (unsigned int cnt=0; cnt < num_commands; cnt++)
            execute (command_list.getCommandString(cnt));

    }
    logdbg  << "SQLiteConnection: execute: end";

    return dbresult;
}

void SQLiteConnection::execute (const std::string &command)
{
    logdbg  << "SQLiteConnection: execute";

    int result;

    prepareStatement(command.c_str());
    result = sqlite3_step(statement_);

    if (result != SQLITE_DONE)
    {
        logerr <<  "SQLiteConnection: execute: problem while stepping the result: " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        throw std::runtime_error ("SQLiteConnection: execute: problem while stepping the result");
    }

    finalizeStatement();
}

void SQLiteConnection::execute (const std::string &command, std::shared_ptr <Buffer> buffer)
{
    logdbg  << "SQLiteConnection: execute";

    assert (buffer);
    unsigned int num_properties=0;

    const PropertyList &list = buffer->properties();
    num_properties = list.size();

    unsigned int cnt=buffer->size();

    int result;

    prepareStatement(command.c_str());

    // Now step throught the result lines
    for (result = sqlite3_step(statement_); result == SQLITE_ROW; result = sqlite3_step(statement_))
    {
        readRowIntoBuffer (list, num_properties, buffer, cnt);
        cnt++;
    }

    if (result != SQLITE_DONE)
    {
        logerr <<  "SQLiteConnection: execute: problem while stepping the result: " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        throw std::runtime_error ("SQLiteConnection: execute: problem while stepping the result");
    }

    finalizeStatement();
}

void SQLiteConnection::readRowIntoBuffer (const PropertyList &list, unsigned int num_properties,
                                          std::shared_ptr <Buffer> buffer, unsigned int index)
{
    for (unsigned int cnt=0; cnt < num_properties; cnt++)
    {
        const Property &prop=list.at(cnt);

        switch (prop.dataType())
        {
        case PropertyDataType::BOOL:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<bool>(prop.name()).set(index, static_cast<bool> (sqlite3_column_int(statement_, cnt)));
//            else
//                buffer->get<bool>(prop.name()).setNone(index);
            //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
            break;
        case PropertyDataType::UCHAR:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<unsigned char>(prop.name()).set(index, static_cast<unsigned char> (sqlite3_column_int(statement_, cnt)));
//            else
//                buffer->get<unsigned char>(prop.name()).setNone(index);
            //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
            break;
        case PropertyDataType::CHAR:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<char>(prop.name()).set(index, static_cast<signed char> (sqlite3_column_int(statement_, cnt)));
//            else
//                buffer->get<char>(prop.name()).setNone(index);
            //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
            break;
        case PropertyDataType::INT:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<int>(prop.name()).set(index, static_cast<int> (sqlite3_column_int(statement_, cnt)));
//            else
//                buffer->get<int>(prop.name()).setNone(index);
            //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
            break;
        case PropertyDataType::UINT:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<unsigned int>(prop.name()).set(index, static_cast<unsigned int> (sqlite3_column_int(statement_, cnt)));
//            else
//                buffer->get<unsigned int>(prop.name()).setNone(index);
            //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
            break;
        case PropertyDataType::STRING:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<std::string>(prop.name()).set(index, std::string (reinterpret_cast<const char*> (sqlite3_column_text(statement_, cnt))));
//            else
//                buffer->get<std::string>(prop.name()).setNone(index);
            //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
            break;
        case PropertyDataType::FLOAT:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<float>(prop.name()).set(index, static_cast<float> (sqlite3_column_double (statement_, cnt)));
//            else
//                buffer->get<float>(prop.name()).setNone(index);
            //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
            break;
        case PropertyDataType::DOUBLE:
            if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                buffer->get<double>(prop.name()).set(index, static_cast<double> (sqlite3_column_double (statement_, cnt)));
//            else
//                buffer->get<double>(prop.name()).setNone(index);
            //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
            break;
        default:
            logerr  <<  "MySQLppConnection: readRowIntoBuffer: unknown property type";
            throw std::runtime_error ("MySQLppConnection: readRowIntoBuffer: unknown property type");
            break;
        }
    }
}

void SQLiteConnection::prepareStatement (const std::string &sql)
{
    logdbg  << "SQLiteConnection: prepareStatement: sql '" << sql << "'";
    int result;
    const char* remaining_sql;

    // Prepare the select statement
    remaining_sql = NULL;
    result = sqlite3_prepare_v2(db_handle_, sql.c_str(), sql.size(), &statement_, &remaining_sql);
    if (result != SQLITE_OK)
    {
        logerr <<  "SQLiteConnection: execute: error " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        sqlite3_close(db_handle_);
        throw std::runtime_error ("SQLiteConnection: execute: error");
    }

    if (remaining_sql && *remaining_sql != '\0')
    {
        logerr  <<  "SQLiteConnection: execute: there was unparsed sql text: " << remaining_sql;
        throw std::runtime_error ("SQLiteConnection: execute: there was unparsed sql text");
    }
}
void SQLiteConnection::finalizeStatement ()
{
    sqlite3_finalize(statement_);
}

void SQLiteConnection::prepareCommand (const std::shared_ptr<DBCommand> command)
{
    assert (prepared_command_==0);
    assert (command);

    prepared_command_=command;
    prepared_command_done_=false;

    prepareStatement (command->get().c_str());
}
std::shared_ptr <DBResult> SQLiteConnection::stepPreparedCommand (unsigned int max_results)
{
    assert (prepared_command_);
    assert (!prepared_command_done_);

    std::string sql = prepared_command_->get();
    assert (prepared_command_->resultList().size() > 0); // data should be returned

    std::shared_ptr <Buffer> buffer (new Buffer (prepared_command_->resultList()));
    assert (buffer->size() == 0);
    std::shared_ptr <DBResult> dbresult (new DBResult(buffer));

    unsigned int num_properties = buffer->properties().size();
    const PropertyList &list = buffer->properties();

    unsigned int cnt = 0;
    int result;
    bool done=true;

    max_results--;

    // Now step throught the result lines
    for (result = sqlite3_step(statement_); result == SQLITE_ROW; result = sqlite3_step(statement_))
    {
        readRowIntoBuffer (list, num_properties, buffer, cnt);

        if (buffer->size()) // 0 == 1 otherwise
            assert (buffer->size() == cnt+1);

        if (max_results != 0 && cnt >= max_results)
        {
            done=false;
            break;
        }

        ++cnt;
    }

    if (result != SQLITE_ROW && result != SQLITE_DONE)
    {
        logerr <<  "SQLiteConnection: stepPreparedCommand: problem while stepping the result: " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        throw std::runtime_error ("SQLiteConnection: stepPreparedCommand: problem while stepping the result");
    }

    assert (buffer->size() <= max_results+1); // because of max_results--

    if (result == SQLITE_DONE || buffer->size() == 0 || done)
    {
        assert (done);
        logdbg  << "SQLiteConnection: stepPreparedCommand: reading done";
        prepared_command_done_=true;

        buffer->lastOne(true);
    }


    return dbresult;
}
void SQLiteConnection::finalizeCommand ()
{
    assert (prepared_command_ != nullptr);
    sqlite3_finalize(statement_);
    prepared_command_=nullptr; // should be deleted by caller
    prepared_command_done_=true;
}


std::map <std::string, DBTableInfo> SQLiteConnection::getTableInfo ()
{
    loginf << "SQLiteConnection: getTableInfo";

    std::map <std::string, DBTableInfo> info;

    for (auto it : getTableList())
    {
        loginf << "SQLiteConnection: getTableInfo: table " << it;
        info.insert (std::pair<std::string, DBTableInfo> (it, getColumnList(it)));
    }

    return info;
}

std::vector <std::string> SQLiteConnection::getDatabases()
{
    return std::vector <std::string>(); //no databases
}

std::vector <std::string> SQLiteConnection::getTableList()  // buffer of table name strings
{
    std::vector <std::string> tables;

    DBCommand command;
    command.set ("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name DESC;");
    //command.set ("SELECT name FROM sqlite_master WHERE name != 'sqlite_sequence' ORDER BY name DESC;");
    PropertyList list;
    list.addProperty ("name", PropertyDataType::STRING);
    command.list (list);

    std::shared_ptr <DBResult> result = execute(command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = result->buffer();

    unsigned int size = buffer->size();
    std::string table_name;

    for (unsigned int cnt=0; cnt < size; cnt++)
    {
        table_name = buffer->get<std::string>("name").get(cnt);

        if (table_name.find("sqlite") != std::string::npos) //ignore sqlite system tables
            continue;

        tables.push_back(buffer->get<std::string>("name").get(cnt));
    }

    return tables;
}

DBTableInfo SQLiteConnection::getColumnList(const std::string &table) // buffer of column name string, data type
{
    logdbg << "SQLiteConnection: getColumnList: table " << table;

    DBTableInfo table_info (table);

    DBCommand command;
    command.set ("PRAGMA table_info("+table+")");

    //int cid, string name, string type,int notnull, string dflt_value, int pk

    PropertyList list;
    list.addProperty ("cid", PropertyDataType::INT);
    list.addProperty ("name", PropertyDataType::STRING);
    list.addProperty ("type", PropertyDataType::STRING);
    list.addProperty ("notnull", PropertyDataType::INT);
    list.addProperty ("dfltvalue", PropertyDataType::STRING);
    list.addProperty ("pk", PropertyDataType::INT);

    command.list (list);

    std::shared_ptr <DBResult> result = execute(command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = result->buffer();

    for (unsigned int cnt=0; cnt < buffer->size(); cnt++)
    {
        //loginf << "UGA " << table << ": "  << buffer->getString("name").get(cnt);

        assert (buffer->has<std::string>("type"));

        std::string data_type = buffer->get<std::string>("type").get(cnt);

        if (data_type == "BOOLEAN")
            data_type = "BOOL";
        else if (data_type == "TEXT")
            data_type = "STRING";
        else if (data_type == "REAL")
            data_type = "DOUBLE";
        else if (data_type == "INTEGER")
            data_type = "INT";

        assert (buffer->has<std::string>("name"));
        assert (buffer->has<int>("pk"));
        assert (buffer->has<int>("notnull"));

        table_info.addColumn (buffer->get<std::string>("name").get(cnt), data_type,
                              buffer->get<int>("pk").get(cnt) > 0, !buffer->get<int>("notnull").get(cnt), "");
    }

    return table_info;
}

void SQLiteConnection::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "SQLiteFile")
    {
      SavedFile *file = new SavedFile (class_id, instance_id, this);
      assert (file_list_.count (file->name()) == 0);
      file_list_.insert (std::pair <std::string, SavedFile*> (file->name(), file));
    }
    else
        throw std::runtime_error ("SQLiteConnection: generateSubConfigurable: unknown class_id "+class_id );
}

QWidget *SQLiteConnection::widget ()
{
    if (!widget_)
    {
        widget_ = new SQLiteConnectionWidget(*this);
    }

    assert (widget_);
    return widget_;
}

QWidget *SQLiteConnection::infoWidget ()
{
    if (!info_widget_)
    {
        info_widget_ = new SQLiteConnectionInfoWidget(*this);
    }

    assert (info_widget_);
    return info_widget_;
}

std::string SQLiteConnection::status () const
{
    if (connection_ready_)
        return "Ready";
    else
        return "Not connected";
}

std::string SQLiteConnection::identifier () const
{
    assert (connection_ready_);

    return last_filename_;
}

void SQLiteConnection::addFile (const std::string &filename)
{
    if (file_list_.count (filename) != 0)
        throw std::invalid_argument ("SQLiteConnection: addFile: name '"+filename+"' already in use");

    std::string instancename = filename;
    instancename.erase (std::remove(instancename.begin(), instancename.end(), '/'), instancename.end());

    Configuration &config = addNewSubConfiguration ("SQLiteFile", "SQLiteFile"+instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable ("SQLiteFile", "SQLiteFile"+instancename);

    if (widget_)
        widget_->updateFileListSlot();
}

void SQLiteConnection::removeFile (const std::string &filename)
{
    if (file_list_.count (filename) != 1)
        throw std::invalid_argument ("SQLiteConnection: addFile: name '"+filename+"' not in use");

    delete file_list_.at(filename);
    file_list_.erase(filename);

    if (widget_)
        widget_->updateFileListSlot();
}
