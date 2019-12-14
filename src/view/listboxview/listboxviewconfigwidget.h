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

#ifndef LISTBOXVIEWCONFIGWIDGET_H_
#define LISTBOXVIEWCONFIGWIDGET_H_

#include <QtWidgets/QWidget>
#include "dbovariable.h"

class DBOVariableOrderedSetWidget;
class QCheckBox;
class ListBoxView;
class QLineEdit;
class QPushButton;

/**
 * @brief Widget with configuration elements for a ListBoxView
 *
 */
class ListBoxViewConfigWidget : public QWidget
{
    Q_OBJECT

public slots:
    void toggleShowOnlySeletedSlot();
    void toggleUsePresentation();
    void toggleUseOverwrite();
    void showAssociationsSlot();
    /// @brief Called when database view checkbox is un/checked
    //void toggleDatabaseView ();
    void exportSlot();
    void exportDoneSlot(bool cancelled);


signals:
     void exportSignal (bool overwrite);

public:
    /// @brief Constructor
    ListBoxViewConfigWidget (ListBoxView* view, QWidget* parent=nullptr);
    /// @brief Destructor
    virtual ~ListBoxViewConfigWidget();

protected:
    /// Base view
    ListBoxView* view_;
    /// Variable read list widget
    DBOVariableOrderedSetWidget *variable_set_widget_{nullptr};

    QCheckBox* only_selected_check_ {nullptr};
    QCheckBox* presentation_check_ {nullptr};
    QCheckBox* associations_check_ {nullptr};

    QCheckBox* overwrite_check_ {nullptr};

    QPushButton *export_button_ {nullptr};
};

#endif /* LISTBOXVIEWCONFIGWIDGET_H_ */
