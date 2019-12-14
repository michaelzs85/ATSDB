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

#ifndef FILTERMANAGER_H_
#define FILTERMANAGER_H_

#include <map>
#include <vector>
#include <string>
#include <QtCore/QObject>

#include "singleton.h"
#include "configurable.h"

class DBFilter;
class ATSDB;
class FilterManagerWidget;
class DBOVariable;

/**
 * @brief Manages all filters and generates SQL conditions
 *
 * Generates DBFilters from configuration and SensorFilters for all DBOs with data and data sources.
 * GUI classes operate on this class for setting and retrieval of the filter configuration.
 * Other modules can retrieve the filter SQL conditions when a loading process is triggered.
 *
 * \todo Generalize SQL condition w.r.t. RDL schema
 */
class FilterManager : public QObject, public Configurable
{
    Q_OBJECT
signals:
    void changedFiltersSignal ();

public slots:
    void startedSlot ();
    void deleteFilterSlot (DBFilter* filter);

public:
    /// @brief Constructor
    FilterManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb);
    /// @brief Destructor
    virtual ~FilterManager();

    /// @brief Returns the SQL condition for a DBO and sets all used variable names
    std::string getSQLCondition (const std::string& dbo_name,std::vector <DBOVariable*>& filtered_variables);

    /// @brief Returns number of existing filters
    unsigned int getNumFilters ();
    /// @brief Returns filter at a given index
    DBFilter* getFilter (unsigned int index);
    std::vector <DBFilter*>& filters () { return filters_; }

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);

    /// @brief Resets all filters
    void reset ();

    FilterManagerWidget* widget ();

protected:
    /// Database definition, resets if changed
    std::string db_id_;

    FilterManagerWidget* widget_ {nullptr};

    /// Container with all DBFilters
    std::vector <DBFilter*> filters_;

    virtual void checkSubConfigurables ();
};

#endif /* FILTERMANAGER_H_ */
