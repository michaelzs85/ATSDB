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

#ifndef DBOEDITDATASOURCESWIDGET_H
#define DBOEDITDATASOURCESWIDGET_H

#include <QtWidgets/QWidget>
#include "dbobject.h"

class DBObject;
class QVBoxLayout;
class QGridLayout;
class QLabel;
class QPushButton;

class DBOEditDataSourcesWidget : public QWidget
{
    Q_OBJECT
public slots:
    void syncOptionsFromDBSlot();
    void addStoredDSSlot ();

    void syncOptionsFromCfgSlot();

    void selectAllActionsSlot();
    void deselectAllActionsSlot();
    void performActionsSlot();

public:
    DBOEditDataSourcesWidget(DBObject* object, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~DBOEditDataSourcesWidget();

    void update ();

private:
    /// @brief DBObject to be managed
    DBObject* object_ {nullptr};

    QVBoxLayout* config_ds_layout_ {nullptr};
    QPushButton* sync_from_cfg_button_ {nullptr};

    QLabel* action_heading_label_ {nullptr};
    std::string action_heading_;
    QGridLayout* action_layout_ {nullptr};
    DBOEditDataSourceActionOptionsCollection action_collection_;

    QPushButton* select_all_actions_ {nullptr};
    QPushButton* deselect_all_actions_ {nullptr};
    QPushButton* perform_actions_button_ {nullptr};

    QGridLayout* db_ds_layout_ {nullptr};
    QPushButton* sync_from_db_button_ {nullptr};

    void clearSyncOptions();
    void displaySyncOptions ();

    void updateActionButtons();
    bool haveActionsToPerform ();
};

#endif // DBOEDITDATASOURCESWIDGET_H
