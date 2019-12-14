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

#ifndef METADBTABLEEDITWIDGET_H_
#define METADBTABLEEDITWIDGET_H_

#include <QtWidgets/QWidget>
#include <map>

class MetaDBTable;
class QLineEdit;
class QComboBox;
class QGridLayout;
class QScrollArea;
class QPushButton;

/**
 * @brief Edit widget for a MetaDBTable
 */
class MetaDBTableWidget: public QWidget
{
    Q_OBJECT

signals:
    /// @brief Emitted when changed
    void changedMetaTable();

public slots:
    /// @brief Sets name
    void editNameSlot (const QString &text);
    /// @brief Sets info
    void editInfoSlot (const QString &text);

    /// @brief Adds new sub meta table
    void addSubTableSlot ();
    /// @brief Updates sub tables grid
    void updateSubTablesGridSlot ();
    void updateColumnsGridSlot ();
    /// @brief Updates meta table selection for new meta sub table
    void updateNewSubTableSelectionSlot();
    /// @brief Updates local key selection for new meta sub table
    void updateLocalKeySelectionSlot ();
    /// @brief Updates sub key selection for new meta sub table
    void updateSubKeySelectionSlot ();

    void lock ();

public:
    /// @brief Constructor
    MetaDBTableWidget(MetaDBTable& meta_table, QWidget* parent=0, Qt::WindowFlags f=0);
    /// @brief Destructor
    virtual ~MetaDBTableWidget();

protected:
    /// Represented meta table
    MetaDBTable& meta_table_;

    /// Name edit field
    QLineEdit* name_edit_ {nullptr};
    /// Info edit field
    QLineEdit* info_edit_ {nullptr};
    /// Key selection field
    //QComboBox* key_box_ {nullptr};

    /// Grid with all sub meta tables
    QGridLayout* sub_tables_grid_ {nullptr};

    QGridLayout* column_grid_ {nullptr};

    /// New sub meta table local key selection
    QComboBox* new_local_key_ {nullptr};
    /// New sub meta table meta table selection
    QComboBox* new_table_ {nullptr};
    /// New sub meta table sub key selection
    QComboBox* new_sub_key_ {nullptr};

    QPushButton* add_button_ {nullptr};

    bool locked_ {false};
};

#endif /* METADBTABLEEDITWIDGET_H_ */
