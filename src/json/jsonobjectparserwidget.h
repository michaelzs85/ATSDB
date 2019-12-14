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

#ifndef JSONOBJECTPARSERWIDGET_H
#define JSONOBJECTPARSERWIDGET_H

#include <QtWidgets/QWidget>

class JSONObjectParser;
class QLineEdit;
class QCheckBox;
class QGridLayout;
class DataTypeFormatSelectionWidget;

class JSONObjectParserWidget : public QWidget
{
    Q_OBJECT
public slots:
    void jsonContainerKeyChangedSlot();
    void jsonKeyChangedSlot ();
    void jsonValueChangedSlot ();

    void overrideDataSourceChangedSlot ();
    void dataSourceVariableChangedSlot ();

    void addNewMappingSlot();

    void mappingActiveChangedSlot();
    void mappingKeyChangedSlot();
    void mappingCommentChangedSlot();
    void mappingDBOVariableChangedSlot();
    void mappingMandatoryChangedSlot();
    void mappingInArrayChangedSlot();
    void mappingAppendChangedSlot();
    void mappingDeleteSlot();

public:
    explicit JSONObjectParserWidget(JSONObjectParser& parser, QWidget *parent = nullptr);

    void setParser (JSONObjectParser& parser);
    void updateMappingsGrid();

private:
    JSONObjectParser* parser_ {nullptr};

    QLineEdit* json_container_key_edit_ {nullptr};  // location of container with target report data
    QLineEdit* json_key_edit_ {nullptr}; // * for all
    QLineEdit* json_value_edit_ {nullptr};

    QCheckBox* override_data_source_check_ {nullptr};
    QLineEdit* data_source_variable_name_edit_ {nullptr};

    QGridLayout* mappings_grid_ {nullptr};

    void update ();
};

#endif // JSONOBJECTPARSERWIDGET_H
