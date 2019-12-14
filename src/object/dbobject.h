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

#ifndef DBOBJECT_H_
#define DBOBJECT_H_

#include <string>
#include <QtCore/QObject>
#include <memory>

#include "global.h"
#include "dbovariableset.h"
#include "dbodatasource.h"
#include "dbodatasourcedefinition.h"
#include "storeddbodatasource.h"
#include "dboeditdatasourceactionoptions.h"
#include "configurable.h"
#include "dbovariable.h"
#include "dboschemametatabledefinition.h"
#include "dboassociationentry.h"

class PropertyList;
class MetaDBTable;

class DBObjectWidget;
class DBObjectInfoWidget;
class DBOEditDataSourcesWidget;
class Buffer;
class Job;
class DBOReadDBJob;
class InsertBufferDBJob;
class UpdateBufferDBJob;
class FinalizeDBOReadJob;
class DBOVariableSet;
class DBOLabelDefinition;
class DBOLabelDefinitionWidget;
class DBObjectManager;

using DBOEditDataSourceActionOptionsCollection = typename std::map<unsigned int, DBOEditDataSourceActionOptions>;

/**
 * @brief Abstract data description of an object stored in a database
 *
 * A database object serves as definition of a data container stored in a database. It is identified by name and a type,
 * and mainly consists of one or a number of meta tables. One meta table is considered the main meta table, all others
 * are sub meta tables. Columns from all such meta tables are collected and abstracted as DBOVariables of an DBObject.
 *
 * The meta table information is depended on the DBSchema, since for different schemas different table structures might exist.
 * With such a construct it is possible to abstract from different database schemas, by creating one set of DBObjects, which
 * are based on different meta tables (depended on the used schema). All DBOVariable instances have a different table variable,
 * also based on the schema.
 *
 * From an outside view, a DBObject is a collection of DBOVariables. However, a specialization exists, which is called the
 * meta DBObject (of type DBO_UNKNOWN), which serves only as an collection of meta DBOVariables and does not have a meta table.
 *
 * The distinction between meta DBOjects and normal ones can be made using the is_meta_ flag or is_loadable_ flag.
 *
 * For loadable DBObjects, a read list is held. This list contains the information, which variables are to be loaded on
 * read statements from the database. This read list is only held, and other classes can add variables to the retrieved
 * reference of the list.
 *
 * Also holds functionality about its data sources. If such information is present (marked in the database schema),
 * such information can be generated and is executed asynchronously. Interested instances have to be registered as observer
 * and receive a callback once the information is present.
 *
 * \todo Check if DBOVariables can exist only in some schemas, finish checkVariables
 */
class DBObject : public QObject, public Configurable
{
    Q_OBJECT
signals:
    void newDataSignal (DBObject& object);
    void loadingDoneSignal (DBObject& object);

    void insertProgressSignal (float percent);
    void insertDoneSignal (DBObject& object);

    void updateProgressSignal (float percent);
    void updateDoneSignal (DBObject& object);

    void labelDefinitionChangedSignal ();

public slots:
    void schemaChangedSlot ();

    void readJobIntermediateSlot (std::shared_ptr<Buffer> buffer);
    void readJobObsoleteSlot ();
    void readJobDoneSlot();
    void finalizeReadJobDoneSlot();

    void insertProgressSlot (float percent);
    void insertDoneSlot ();

    void updateProgressSlot (float percent);
    void updateDoneSlot ();

    void dataSourceDefinitionChanged ();

public:
    /// @brief Constructor
    DBObject(const std::string& class_id, const std::string& instance_id, DBObjectManager* manager);
    /// @brief Desctructor
    virtual ~DBObject();

    /// @brief Returns flag indication if a DBOVariable identified by id exists
    bool hasVariable (const std::string& name) const;
    /// @brief Returns variable identified by id
    DBOVariable& variable (const std::string& name);
    void renameVariable (const std::string& name, const std::string& new_name);
    /// @brief Deletes a variable identified by id
    void deleteVariable (const std::string& name);

    bool uses (const DBTableColumn& column) const;

    using DBOVariableIterator = typename std::map<std::string, DBOVariable>::iterator;
    DBOVariableIterator begin() { return variables_.begin(); }
    DBOVariableIterator end() { return variables_.end(); }

    /// @brief Returns number of existing variables
    size_t numVariables () const { return variables_.size(); }

    /// @brief Returns name of the object
    const std::string& name () const { return name_; }
    /// @brief Sets name of the object
    void name (const std::string& name) { assert (name.size() > 0); name_=name; }

    /// @brief Returns description of the object
    const std::string& info () const { return info_; }
    /// @brief Sets description of the object
    void info(const std::string& info) { info_=info; }

    /// @brief Returns if an object can be loaded
    bool loadable () const { return is_loadable_; }

    void loadingWanted (bool wanted) { loading_wanted_=wanted; }
    bool loadingWanted () { return loading_wanted_; }

    void load (DBOVariableSet& read_set, bool use_filters, bool use_order, DBOVariable* order_variable,
               bool use_order_ascending, const std::string& limit_str="");
    void load (DBOVariableSet& read_set, std::string custom_filter_clause,
               std::vector <DBOVariable*> filtered_variables, bool use_order, DBOVariable* order_variable,
               bool use_order_ascending, const std::string& limit_str="");
    void quitLoading ();
    void clearData ();

    // takes buffers with dbovar names & datatypes & units, converts itself
    void insertData (DBOVariableSet& list, std::shared_ptr<Buffer> buffer, bool emit_change=true);
    // takes buffers with dbovar names & datatypes & units, converts itself
    void updateData (DBOVariable &key_var, DBOVariableSet& list, std::shared_ptr<Buffer> buffer);

    std::map<int, std::string> loadLabelData (std::vector<int> rec_nums, int break_item_cnt);

    /// @brief Returns if incremental read for DBO type was prepared
    bool isLoading ();
    /// @brief Returns if DBO exists and has data in the database
    bool hasData ();
    /// @brief Returns number of elements for DBO type
    size_t count ();
    size_t loadedCount ();

    /// @brief Returns container with all meta tables
    const std::map <std::string, DBOSchemaMetaTableDefinition>& metaTables () const {
        return meta_table_definitions_; }
    /// @brief Returns identifier of main meta table under DBSchema defined by schema
    bool hasMetaTable (const std::string& schema) const;
    const std::string& metaTable (const std::string& schema) const;
    void deleteMetaTable (const std::string& schema);

    /// @brief Returns main meta table for current schema
    MetaDBTable& currentMetaTable () const;
    /// @brief Returns if current schema has main meta table
    bool hasCurrentMetaTable () const;

    /// @brief Returns if a data source is defined in the current schema
    bool hasCurrentDataSourceDefinition () const;
    /// @brief Returns current data source definition
    const DBODataSourceDefinition& currentDataSourceDefinition () const;
    bool hasDataSourceDefinition (const std::string& schema) { return data_source_definitions_.count(schema); }
    void deleteDataSourceDefinition (const std::string& schema);
    /// @brief Returns container with all data source definitions
    std::map <std::string, DBODataSourceDefinition>& dataSourceDefinitions () {
        return data_source_definitions_;
    }

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    bool hasKeyVariable ();
    DBOVariable& getKeyVariable();

    using StoredDataSourceIterator = typename std::map<unsigned int, StoredDBODataSource>::iterator;
    StoredDataSourceIterator storedDSBegin() { return stored_data_sources_.begin(); }
    StoredDataSourceIterator storedDSEnd() { return stored_data_sources_.end(); }

    /// @brief Returns flag indication if a StoredDBODataSource identified by name exists
    bool hasStoredDataSource (unsigned int id) const;
    /// @brief Returns variable identified by id
    StoredDBODataSource& storedDataSource (unsigned int id);
    StoredDBODataSource& addNewStoredDataSource ();
//    /// @brief Deletes a variable identified by id
    void deleteStoredDataSource (unsigned int id);

    using DataSourceIterator = typename std::map<int, DBODataSource>::iterator;
    DataSourceIterator dsBegin() { return data_sources_.begin(); }
    DataSourceIterator dsEnd() { return data_sources_.end(); }

    ///@brief Returns flag if data sources are defined for DBO type.
    bool hasDataSources () { return data_sources_.size() > 0; }
    void addDataSource (int key_value, const std::string& name); // needs postprocessing after
    void addDataSources (std::map <int, std::pair<int,int>>& sources);
    bool hasDataSource (int id);
    DBODataSource& getDataSource (int id);
    void updateDataSource (int id);
    ///@brief Returns data source name for a DBO type and data source number.
    const std::string& getNameOfSensor (int id);
    const std::map<int, DBODataSource>& dataSources() const { return data_sources_; }

    DBOEditDataSourceActionOptionsCollection getSyncOptionsFromDB ();
    DBOEditDataSourceActionOptionsCollection getSyncOptionsFromCfg ();

    /// @brief Return if active data sources info is available
    bool hasActiveDataSourcesInfo ();

    /// @brief Returns container with the active data sources information
    const std::set<int> getActiveDataSources ();

    std::string status ();

    DBObjectWidget* widget ();
    DBObjectInfoWidget* infoWidget ();
    DBOLabelDefinitionWidget* labelDefinitionWidget();
    DBOEditDataSourcesWidget* editDataSourcesWidget();

    std::shared_ptr<Buffer> data () { return data_; }

    void lock ();
    void unlock ();

    bool existsInDB () const;

    void print ();

    void removeDependenciesForSchema (const std::string& schema_name);

    // association stuff
    void loadAssociationsIfRequired (); // starts loading job if required
    void loadAssociations (); // actually loads associations, should be called from job
    bool hasAssociations ();
    void addAssociation (unsigned int rec_num, unsigned int utn, unsigned int src_rec_num);
    const DBOAssociationCollection& associations() { return associations_; }
    void clearAssociations ();
    void saveAssociations ();

    void updateToDatabaseContent ();

protected:
    DBObjectManager& manager_;
    /// DBO name
    std::string name_;
    /// DBO description
    std::string info_;
    /// DBO is loadable flag
    bool is_loadable_ {false}; // loadable on its own
    bool loading_wanted_ {false};
    size_t count_ {0};

    std::unique_ptr<DBOLabelDefinition> label_definition_;

    std::shared_ptr <DBOReadDBJob> read_job_ {nullptr};
    std::vector <std::shared_ptr<Buffer>> read_job_data_;
    std::vector <std::shared_ptr <FinalizeDBOReadJob>> finalize_jobs_;

    std::shared_ptr <InsertBufferDBJob> insert_job_ {nullptr};
    std::shared_ptr <UpdateBufferDBJob> update_job_ {nullptr};

    std::shared_ptr<Buffer> data_;

    bool locked_ {false};

    /// Container with all DBOSchemaMetaTableDefinitions
    std::map <std::string, DBOSchemaMetaTableDefinition> meta_table_definitions_;

    std::unique_ptr<DBOEditDataSourcesWidget> edit_ds_widget_;

    /// Container with data source definitions (schema identifier -> data source definition pointer)
    std::map<std::string, DBODataSourceDefinition> data_source_definitions_;
    std::map<unsigned int, StoredDBODataSource> stored_data_sources_;
    std::map<int, DBODataSource> data_sources_;
    /// Container with all variables (variable identifier -> variable pointer)
    std::map<std::string, DBOVariable> variables_;

    /// Current (in the current schema) main meta table
    MetaDBTable* current_meta_table_ {nullptr}; // TODO rework const?
    std::string associations_table_name_;

    std::unique_ptr<DBObjectWidget> widget_;
    std::unique_ptr<DBObjectInfoWidget> info_widget_;

    bool associations_changed_ {false};
    bool associations_loaded_ {false};
    DBOAssociationCollection associations_;

    virtual void checkSubConfigurables ();

    ///@brief Generates data sources information from previous post-processing.
    void buildDataSources();
    void removeVariableInfoForSchema (const std::string& schema_name);
};

#endif /* DBOBJECT_H_ */
