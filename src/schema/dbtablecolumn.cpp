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

#include "dbtable.h"
#include "dbtablecolumn.h"
#include "logger.h"
#include "unitselectionwidget.h"
#include "dbinterface.h"
#include "dbtableinfo.h"

// TODO watch out for unsigned
std::map<std::string, PropertyDataType> DBTableColumn::db_types_2_data_types_ {
        {"bool", PropertyDataType::BOOL},
        {"tinyint", PropertyDataType::CHAR},
        //{""UCHAR", PropertyDataType::UCHAR},
        {"smallint", PropertyDataType::INT},
        {"mediumint", PropertyDataType::INT},
        {"int", PropertyDataType::INT},
        //{""UINT", PropertyDataType::UCHAR},
        {"bigint", PropertyDataType::LONGINT},
        //{""ULONGINT", PropertyDataType::ULONGINT},
        {"float", PropertyDataType::FLOAT},
        {"double", PropertyDataType::DOUBLE},
        {"enum", PropertyDataType::STRING},
        {"tinyblob", PropertyDataType::STRING},
        {"char", PropertyDataType::STRING},
        {"blob", PropertyDataType::STRING},
        {"mediumblob", PropertyDataType::STRING},
        {"longblob", PropertyDataType::STRING},
        {"varchar", PropertyDataType::STRING}
};

DBTableColumn::DBTableColumn(const std::string &class_id, const std::string &instance_id, DBTable *table,
                             DBInterface& db_interface)
 : Configurable (class_id, instance_id, table), table_(*table), db_interface_(db_interface)
{
  registerParameter ("name", &name_, "");
  registerParameter ("type", &type_, "");
  registerParameter ("is_key", &is_key_, false);
  registerParameter ("comment", &comment_, "");
  registerParameter ("dimension", &dimension_, "");
  registerParameter ("unit", &unit_, "");
  registerParameter ("special_null", &special_null_, "");

  identifier_ = table_.name()+"."+name_;

  createSubConfigurables();
}

DBTableColumn::~DBTableColumn()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

}

void DBTableColumn::name (const std::string &name)
{
    name_=name;
    table_.name()+"."+name_;
}

PropertyDataType DBTableColumn::propertyType () const
{
//        BOOL, CHAR, UCHAR, INT, UINT, LONGINT, ULONGINT, FLOAT, DOUBLE, STRING
    if (db_types_2_data_types_.count(type_) == 0)
        return Property::asDataType(type_);
    else
        return db_types_2_data_types_.at(type_);
}


UnitSelectionWidget* DBTableColumn::unitWidget ()
{
    if (!widget_)
    {
        widget_ = new UnitSelectionWidget (dimension_, unit_);
        assert (widget_);
    }

    return widget_;
}

void DBTableColumn::updateOnDatabase()
{
    exists_in_db_ = false;

    const std::map <std::string, DBTableInfo> &all_table_infos = db_interface_.tableInfo ();

    std::string table_name = table_.name();

    if (all_table_infos.count(table_name) != 0)
    {
        const DBTableInfo& table_info = all_table_infos.at(table_name);

        if (table_info.hasColumn(name_))
            exists_in_db_ = true;
    }

    loginf << "DBTableColumn: updateOnDatabase: table " <<  table_name << " column "
           << name_ << " exists in db " << exists_in_db_;
}
