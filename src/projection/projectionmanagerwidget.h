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

#ifndef PROJECTIONMANAGERWIDGET_H_
#define PROJECTIONMANAGERWIDGET_H_

#include <QtWidgets/QWidget>

class QLineEdit;
class QLabel;
class QRadioButton;

class ProjectionManager;

class ProjectionManagerWidget : public QWidget
{
    Q_OBJECT

public slots:
    void projectionChangedSlot();
    void changedEPSGSlot();

public:
    ProjectionManagerWidget(ProjectionManager& proj_man, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~ProjectionManagerWidget();

protected:
    ProjectionManager& projection_manager_;

    QRadioButton* ogr_radio_ {nullptr};
    QLabel* world_proj_info_label_ {nullptr};
    QLineEdit* epsg_edit_ {nullptr};
    QLabel* cart_proj_info_label_ {nullptr};

    QRadioButton* sdl_radio_ {nullptr};
    QRadioButton* rs2g_radio_ {nullptr};
};


#endif /* PROJECTIONMANAGERWIDGET_H_ */
