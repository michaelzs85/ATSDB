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

#ifndef DBOBJECTMANAGERWIDGET_H_
#define DBOBJECTMANAGERWIDGET_H_

#include <QtCore/QObject>
#include <QtWidgets/QFrame>
#include <map>

class DBObject;
class DBObjectWidget;
class DBObjectManager;
class DBSchemaManager;
class MetaDBOVariable;
class MetaDBOVariableWidget;
class QGridLayout;
class QScrollArea;
class QPushButton;
class QLineEdit;
class QComboBox;

/**
 * @brief Shows all DBObjects, allows editing and adding new ones
 */
class DBObjectManagerWidget : public QFrame
{
    Q_OBJECT

public slots:
    /// @brief Adds a DBObject
    void addDBOSlot ();
    /// @brief Is called when a DBObject was changed
    void changedDBOSlot ();
    /// @brief Edits a DBObject
    void editDBOSlot ();
    /// @brief Deletes a DBObject
    void deleteDBOSlot ();
    /// @brief Updates the DBObject list
    void updateDBOsSlot ();

    void addMetaVariableSlot ();
    void editMetaVariableSlot ();
    void deleteMetaVariableSlot ();
    void addAllMetaVariablesSlot ();
    void updateMetaVariablesSlot ();

    /// @brief Unlocks editing functionality
    void databaseOpenedSlot ();

public:
    /// @brief Constructor
    DBObjectManagerWidget(DBObjectManager &object_manager);
    /// @brief Destructor
    virtual ~DBObjectManagerWidget();

    void lock ();
    void unlock ();

private:
    DBObjectManager& object_manager_;
    DBSchemaManager& schema_manager_;
    /// Grid with all DBObjects
    QGridLayout* dbobjects_grid_ {nullptr};
    QGridLayout* meta_variables_grid_ {nullptr};

    /// Editing functionality unlocked flag
    bool locked_ {false};

    /// New DBO add button
    QPushButton* add_dbo_button_ {nullptr};
    QPushButton* add_metavar_button_ {nullptr};

    /// Container with DBO edit buttons
    std::map <QPushButton*, DBObject*> edit_dbo_buttons_;
    /// Container with DBO edit buttons
    std::map <QPushButton*, DBObject*> delete_dbo_buttons_;

    /// Container with already existing edit DBO widgets
    std::map <DBObject*, DBObjectWidget*> edit_dbo_widgets_;

    std::map <QPushButton*, MetaDBOVariable*> edit_meta_buttons_;
    /// Container with DBO edit buttons
    std::map <QPushButton*, MetaDBOVariable*> delete_meta_buttons_;
    std::map <MetaDBOVariable*, MetaDBOVariableWidget*> edit_meta_widgets_;
};

#endif /* DBOBJECTMANAGERWIDGET_H_ */
