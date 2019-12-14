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


#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "atsdb.h"
#include "listboxviewdatasource.h"
#include "logger.h"
#include "job.h"


#include <QtWidgets/QMessageBox>

ListBoxViewDataSource::ListBoxViewDataSource(const std::string& class_id, const std::string& instance_id,
                                             Configurable* parent)
    : QObject(), Configurable (class_id, instance_id, parent),
      selection_entries_ (ViewSelection::getInstance().getEntries())
{
    //registerParameter ("use_selection", &use_selection_, true);

    connect (&ATSDB::instance().objectManager(), SIGNAL(loadingStartedSignal()), this, SLOT(loadingStartedSlot()));

    for (auto& obj_it : ATSDB::instance().objectManager())
    {
        connect (obj_it.second, SIGNAL (newDataSignal(DBObject&)), this, SLOT(newDataSlot(DBObject&)));
        connect (obj_it.second, SIGNAL (loadingDoneSignal(DBObject &)), this, SLOT(loadingDoneSlot(DBObject&)));
    }

    createSubConfigurables ();
}

ListBoxViewDataSource::~ListBoxViewDataSource()
{
    if (set_)
    {
        delete set_;
        set_ = nullptr;
    }
}

void ListBoxViewDataSource::generateSubConfigurable (const std::string& class_id, const std::string& instance_id)
{
    logdbg  << "ListBoxViewDataSource: generateSubConfigurable: class_id " << class_id << " instance_id "
            << instance_id;

    if (class_id.compare("DBOVariableOrderedSet") == 0)
    {
        assert (set_ == 0);
        set_ = new DBOVariableOrderedSet (class_id, instance_id, this);
    }
    else
        throw std::runtime_error ("ListBoxViewDataSource: generateSubConfigurable: unknown class_id "+class_id );
}

void ListBoxViewDataSource::checkSubConfigurables ()
{
    if (set_ == nullptr)
    {
        generateSubConfigurable ("DBOVariableOrderedSet", "DBOVariableOrderedSet0");
        assert (set_);

        DBObjectManager& obj_man = ATSDB::instance().objectManager();

        if (obj_man.existsMetaVariable("rec_num"))
            set_->add (obj_man.metaVariable("rec_num"));

        //        Time of Day
        if (obj_man.existsMetaVariable("tod"))
            set_->add (obj_man.metaVariable("tod"));

        //        Datasource
        if (obj_man.existsMetaVariable("ds_id"))
            set_->add (obj_man.metaVariable("ds_id"));

        //        Lat/Long
        if (obj_man.existsMetaVariable("pos_lat_deg"))
            set_->add (obj_man.metaVariable("pos_lat_deg"));
        if (obj_man.existsMetaVariable("pos_long_deg"))
            set_->add (obj_man.metaVariable("pos_long_deg"));

        //        Mode 3/A code
        if (obj_man.existsMetaVariable("mode3a_code"))
            set_->add (obj_man.metaVariable("mode3a_code"));

        //        Mode S TA
        if (obj_man.existsMetaVariable("target_addr"))
            set_->add (obj_man.metaVariable("target_addr"));

        //        Mode S Callsign
        if (obj_man.existsMetaVariable("callsign"))
            set_->add (obj_man.metaVariable("callsign"));

        //        Mode C
        if (obj_man.existsMetaVariable("modec_code_ft"))
            set_->add (obj_man.metaVariable("modec_code_ft"));

        //        Track Number
        if (obj_man.existsMetaVariable("track_num"))
            set_->add (obj_man.metaVariable("track_num"));
    }
}

void ListBoxViewDataSource::loadingStartedSlot ()
{
    logdbg << "ListBoxViewDataSource: loadingStartedSlot";
    emit loadingStartedSignal ();
}

void ListBoxViewDataSource::newDataSlot (DBObject &object)
{
    logdbg << "ListBoxViewDataSource: newDataSlot: object " << object.name();

    std::shared_ptr <Buffer> buffer = object.data();
    assert (buffer);

    emit updateData (object, buffer);
}

void ListBoxViewDataSource::loadingDoneSlot(DBObject &object)
{
    logdbg << "ListBoxViewDataSource: loadingDoneSlot: object " << object.name();
}

