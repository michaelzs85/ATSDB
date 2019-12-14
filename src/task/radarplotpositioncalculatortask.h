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

#ifndef RADARPLOTPOSITIONCALCULATOR_H_
#define RADARPLOTPOSITIONCALCULATOR_H_

#include "configurable.h"
#include "dbodatasource.h"

#include <QtCore/QObject>
#include <memory>

class Buffer;
class DBObject;
class DBOVariable;
class RadarPlotPositionCalculatorTaskWidget;
class TaskManager;
class UpdateBufferDBJob;

class QMessageBox;

class RadarPlotPositionCalculatorTask : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    //void newDataSlot (DBObject &object);
    void newDataSlot (DBObject& object);
    void loadingDoneSlot (DBObject& object);

    void updateProgressSlot (float percent);
    void updateDoneSlot (DBObject& object);

    //void updateBufferJobStatusSlot ();

public:
    RadarPlotPositionCalculatorTask(const std::string& class_id, const std::string& instance_id,
                                    TaskManager* task_manager);
    virtual ~RadarPlotPositionCalculatorTask();

    bool canCalculate ();
    void calculate ();

    bool isCalculating ();
    unsigned int getNumLoaded () { return num_loaded_; }

    bool hasOpenWidget() { return widget_ != nullptr; }
    RadarPlotPositionCalculatorTaskWidget* widget();

    std::string dbObjectStr() const;
    void dbObjectStr(const std::string& db_object_str);

    std::string keyVarStr() const;
    void keyVarStr(const std::string& key_var_str);

    std::string datasourceVarStr() const;
    void datasourceVarStr(const std::string& datasource_var_str);

    std::string rangeVarStr() const;
    void rangeVarStr(const std::string& range_var_str);

    std::string azimuthVarStr() const;
    void azimuthVarStr(const std::string& azimuth_var_str);

    std::string altitudeVarStr() const;
    void altitudeVarStr(const std::string& altitude_var_str);

    std::string latitudeVarStr() const;
    void latitudeVarStr(const std::string& latitude_var_str);

    std::string longitudeVarStr() const;
    void longitudeVarStr(const std::string& longitude_var_str);

protected:
    std::string db_object_str_;
    DBObject* db_object_{nullptr};

    std::string key_var_str_;
    DBOVariable* key_var_{nullptr};

    std::string datasource_var_str_;
    DBOVariable* datasource_var_{nullptr};

    std::string range_var_str_;
    DBOVariable* range_var_{nullptr};

    std::string azimuth_var_str_;
    DBOVariable* azimuth_var_{nullptr};

    std::string altitude_var_str_;
    DBOVariable* altitude_var_{nullptr};

    std::string latitude_var_str_;
    DBOVariable* latitude_var_{nullptr};

    std::string longitude_var_str_;
    DBOVariable* longitude_var_{nullptr};

    std::shared_ptr<UpdateBufferDBJob> job_ptr_;

    bool calculating_ {false};
    bool calculated_ {false};

    unsigned int num_loaded_ {0};

    RadarPlotPositionCalculatorTaskWidget* widget_ {nullptr};

    QMessageBox* msg_box_ {nullptr};
    size_t target_report_count_{0};

    void checkAndSetVariable (std::string &name_str, DBOVariable** var);
};

#endif /* RADARPLOTPOSITIONCALCULATOR_H_ */
