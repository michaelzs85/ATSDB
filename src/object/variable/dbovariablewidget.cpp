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

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTextEdit>

#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbovariablewidget.h"
#include "dbovariable.h"
#include "dbtablecolumn.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "metadbtable.h"
#include "logger.h"
#include "dbovariabledatatypecombobox.h"
#include "stringrepresentationcombobox.h"
#include "dbtablecolumncombobox.h"
#include "unitselectionwidget.h"
#include "atsdb.h"

#include "stringconv.h"

using namespace Utils;

DBOVariableWidget::DBOVariableWidget(DBOVariable& variable, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), variable_(&variable)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Edit DB object variable");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    // object parameters
    properties_layout_ = new QGridLayout ();

    unsigned int row=0;
    QLabel *name_label = new QLabel ("Name");
    properties_layout_->addWidget(name_label, row, 0);

    name_edit_ = new QLineEdit (variable_->name().c_str());
    connect(name_edit_, SIGNAL( returnPressed() ), this, SLOT( editNameSlot() ));
    properties_layout_->addWidget (name_edit_, row, 1);
    row++;

    QLabel *description_label = new QLabel ("Description");
    properties_layout_->addWidget(description_label, row, 0);

    description_edit_ = new QLineEdit (variable_->description().c_str());
    connect(description_edit_, SIGNAL( returnPressed() ), this, SLOT( editDescriptionSlot() ));
    properties_layout_->addWidget (description_edit_, row, 1);
    row++;

    QLabel *type_label = new QLabel ("Data Type");
    properties_layout_->addWidget(type_label, row, 0);

    type_combo_ = new DBOVariableDataTypeComboBox (*variable_);
    connect (type_combo_, SIGNAL(changedType()), this, SLOT(editDataTypeSlot()));
    properties_layout_->addWidget (type_combo_, row, 1);
    row++;

    properties_layout_->addWidget(new QLabel ("Representation"), row, 0);

    representation_box_ = new StringRepresentationComboBox (*variable_);
    properties_layout_->addWidget (representation_box_, row, 1);
    row++;

    QLabel *unit_label = new QLabel ("Unit");
    properties_layout_->addWidget(unit_label, row, 0);

    unit_sel_ = new UnitSelectionWidget (variable_->dimension(), variable_->unit());
    properties_layout_->addWidget (unit_sel_, row, 1);
    row++;

    schema_boxes_row_ = row;
    createSchemaBoxes ();

    main_layout->addLayout (properties_layout_);
    main_layout->addStretch();

    setLayout (main_layout);

    show();
}

DBOVariableWidget::~DBOVariableWidget()
{

}

void DBOVariableWidget::lock ()
{
    if (locked_)
        return;

    name_edit_->setDisabled (true);
    description_edit_->setDisabled (true);
    type_combo_->setDisabled (true);
    representation_box_->setDisabled (true);
    unit_sel_->setDisabled (true);

    for (auto& box_it : schema_boxes_)
        box_it.second->setDisabled (true);

    locked_ = true;
}

void DBOVariableWidget::unlock ()
{
    if (!locked_)
        return;

    name_edit_->setDisabled (false);
    description_edit_->setDisabled (false);
    type_combo_->setDisabled (false);
    representation_box_->setDisabled (false);
    unit_sel_->setDisabled (false);

    for (auto& box_it : schema_boxes_)
        box_it.second->setDisabled (false);

    locked_ = false;
}

void DBOVariableWidget::setVariable (DBOVariable& variable)
{
    variable_ = &variable;

    update ();
}

void DBOVariableWidget::update ()
{
    name_edit_->setText(variable_->name().c_str());
    description_edit_->setText(variable_->description().c_str());
    type_combo_->setVariable(*variable_);
    representation_box_->setVariable(*variable_);
    unit_sel_->update(variable_->dimension(), variable_->unit());

    createSchemaBoxes ();
}

void DBOVariableWidget::editNameSlot ()
{
    logdbg  << "DBOVariableWidget: editName";
    assert (name_edit_);

    std::string text = name_edit_->text().toStdString();
    assert (text.size()>0);
    variable_->name (text);
    emit dboVariableChangedSignal();
}
void DBOVariableWidget::editDescriptionSlot()
{
    logdbg  << "DBOVariableWidget: editDescriptionSlot";
    assert (description_edit_);

    std::string text = description_edit_->text().toStdString();
    assert (text.size()>0);
    variable_->description (text);
    emit dboVariableChangedSignal();
}

void DBOVariableWidget::editDataTypeSlot()
{
    logdbg  << "DBOVariableWidget: editDataTypeSlot";
    assert (type_combo_);
    variable_->dataType(type_combo_->getType());
    emit dboVariableChangedSignal();

}

void DBOVariableWidget::createSchemaBoxes ()
{
    loginf << "DBOVariableWidget: createSchemaBoxes";

    auto& meta_tables = variable_->dbObject().metaTables();
    auto& schemas  = ATSDB::instance().schemaManager().getSchemas();

    assert (properties_layout_);
    schema_boxes_.clear();

    int row = schema_boxes_row_;

    std::string schema_name;

    for (auto sit = schemas.begin(); sit != schemas.end(); sit++)
    {
        schema_name = sit->first;

        if (meta_tables.find (schema_name) == meta_tables.end())
            continue;

        std::string schema_string = "Schema: "+schema_name;
        QLabel *label = new QLabel (schema_string.c_str());
        properties_layout_->addWidget(label, row, 0);

        assert (meta_tables.count(schema_name) == 1);
        DBTableColumnComboBox* box = new DBTableColumnComboBox (schema_name, meta_tables.at(schema_name).metaTable(),
                                                                *variable_);

        properties_layout_->addWidget (box, schema_boxes_row_, 1);

        assert (schema_boxes_.count(schema_name) == 0);
        schema_boxes_[schema_name] = box;

        row++;
    }

    loginf  << "DBOVariableWidget: createSchemaBoxes: done";
}
