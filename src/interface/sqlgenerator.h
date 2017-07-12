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

/*
 * SQLGenerator.h
 *
 *  Created on: Jan 11, 2012
 *      Author: sk
 */

#ifndef SQLGENERATOR_H_
#define SQLGENERATOR_H_

#include <memory>

#include "dbovariableset.h"

class Buffer;
class DBCommand;
class DBCommandList;
class DBInterface;
class MetaDBTable;
class DBTableColumn;
class DBObject;

/**
 * @brief Creates SQL statements
 *
 * Returns SQL strings and commands for database functions. All DBCommands and DBCommandLists have to be deleted by the caller.
 *
 * \todo Generalize stuff
 */
class SQLGenerator
{
public:
    /// @brief Constructor
    SQLGenerator(const DBInterface &db_interface);
    /// @brief Destructor
    virtual ~SQLGenerator();

    /// @brief Returns statement to bind variables for buffer contents
//    std::string createDBInsertStringBind(Buffer *buffer, const std::string &tablename);
//    /// @brief Returns statement to bind variables for buffer contents
//    std::string createDBUpdateStringBind(Buffer *buffer, const std::string &tablename);
//    /// @brief Returns statement to create table for buffer contents
//    std::string createDBCreateString (Buffer *buffer, const std::string &tablename);

    /// @brief Returns general select statement
    std::shared_ptr<DBCommand> getSelectCommand (const DBObject &object, const PropertyList &variables,
            const std::string &filter, const std::vector <std::string> &filtered_variable_names, DBOVariable *order=nullptr,
            const std::string &limit="", bool distinct=false, bool left_join=false);

//    /// @brief Returns general select command
//    std::shared_ptr <DBCommand> getSelectCommand(const DBObject &object, const DBOVariableSet &read_list, const std::string &custom_filter_clause,
//                                                 std::vector<std::string> &filtered_variable_names, DBOVariable *order=0, const std::string &limit_str="");
//    /// @brief Returns command for all data sources select

//    //DBCommand *getDataSourcesSelectCommand (const std::string &dbo_type);
//    /// @brief Returns command for active data sources select
//    //DBCommand *getDistinctDataSourcesSelectCommand (const std::string &dbo_type);

//    DBCommand *getDistinctStatistics (const std::string &dbo_type, DBOVariable *variable, unsigned int sensor_number);

//    /// @brief Returns minimum/maximum table name
//    const std::string &getMinMaxTableName () { return table_name_minxmax_; }
//    /// @brief Returns properties table name
//    const std::string &getPropertiesTableName () { return table_name_properties_; }

//    /// @brief Returns statement to check table existence
//    std::string getContainsStatement (const std::string &table_name);
    /// @brief Returns statement to query number of records
    std::string getCountStatement (const std::string &table);
    //DBCommand *getCountStatement (const DBObject &object, unsigned int sensor_number);

//    /// @brief Returns minimum/maximum table creation statement
//    std::string getTableMinMaxCreateStatement ();
//    /// @brief Returns properties table creation statement
//    std::string getTablePropertiesCreateStatement ();

//    /// @brief Returns property insertion statement
//    std::string getInsertPropertyStatement (const std::string &id, const std::string &value);
//    /// @brief Returns property selection statement
//    std::string getSelectPropertyStatement (const std::string &id);

//    /// @brief Returns minimum/maximum insertion statement
//    std::string getInsertMinMaxStatement (const std::string &id, const std::string &dbo_type, std::string min, const std::string &max);
//    /// @brief Returns minimum/maximum selection statement
//    std::string getSelectMinMaxStatement (const std::string &id, const std::string &dbo_type);
//    std::string getSelectMinMaxStatement ();

//    /// @brief Returns general info select statement
//    DBCommand *getSelectInfoCommand(const std::string &dbo_type, std::vector<unsigned int> ids, DBOVariableSet read_list,
//            bool use_filters, std::string order_by_variable, bool ascending, unsigned int limit_min=0,
//            unsigned int limit_max=0);
//    DBCommand *getTableSelectMinMaxNormalStatement (const std::string &table_name);
//    DBCommand *getColumnSelectMinMaxStatement (DBTableColumn *column, std::string table_name);

//    std::string getDeleteStatement (const DBTableColumn &column, const std::string &value, const std::string &filter);
//    std::string getUpdateStatement (const DBTableColumn &column, const std::string &value, const std::string &new_value, const std::string &filter);

//    /// @brief Minimum/Maximum select statement for one variable in one table
//    std::string getMinMaxSelectStatement (const std::string &variable, const std::string& table, const std::string &condition="");
//    /// @brief Minimum/Maximum select statment for a number of variables in one table
//    std::string getMinMaxSelectStatements (const std::vector <std::string> &variables, const std::string &table);
//    std::string getDistinctSelectStatement (const DBTableColumn &column, const std::string &filter_condition="");
//    std::string getShowDatabasesStatement ();

protected:
    /// Pointer to used DBInterface
    const DBInterface &db_interface_;
    /// Flag if db_type_ was set
    bool db_type_set_;
    /// Database connection type
    //DB_CONNECTION_TYPE db_type_;

    /// Properties table name
    std::string table_name_properties_;
    /// Minimum/maximum table name
    std::string table_name_minxmax_;

    /// Minimum/maximum table create SQL statement
    std::string table_minmax_create_statement_;
    /// Properties table create SQL statement
    std::string table_properties_create_statement_;

    /// @brief Returns SQL where clause with all used meta sub-tables
    std::string subTablesWhereClause (const MetaDBTable &meta_table, const std::vector <std::string> &used_tables);
    /// @brief Returns SQL key clause for a give meta sub-table
    std::string subTableKeyClause (const MetaDBTable &meta_table, const std::string &sub_table_name);
};

#endif /* SQLGENERATOR_H_ */
