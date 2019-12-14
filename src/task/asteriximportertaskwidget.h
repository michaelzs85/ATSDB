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

#ifndef ASTERIXIMPORTERTASKWIDGET_H
#define ASTERIXIMPORTERTASKWIDGET_H

#include <QtWidgets/QWidget>
#include <memory>

#include <jasterix/jasterix.h>

class ASTERIXImporterTask;
class ASTERIXConfigWidget;

class QHBoxLayout;
class QPushButton;
class QListWidget;
class QStackedWidget;
class QCheckBox;

class ASTERIXImporterTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void addFileSlot ();
    void deleteFileSlot ();
    void selectedFileSlot ();
    void updateFileListSlot ();

    void addObjectParserSlot ();
    void removeObjectParserSlot ();
    void selectedObjectParserSlot ();

    void debugChangedSlot ();
    void limitRAMChangedSlot ();
    void createMappingsSlot();
    void testImportSlot();
    void importSlot();

public:
    ASTERIXImporterTaskWidget(ASTERIXImporterTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~ASTERIXImporterTaskWidget();

    void importDone ();

protected:
    ASTERIXImporterTask& task_;

    QHBoxLayout *main_layout_ {nullptr};

    QListWidget* file_list_ {nullptr};
    QPushButton* add_file_button_ {nullptr};
    QPushButton* delete_file_button_ {nullptr};

    QListWidget* object_parser_list_ {nullptr};
    QPushButton* add_object_parser_button_ {nullptr};
    QPushButton* delete_object_parser_button_ {nullptr};

    QStackedWidget* object_parser_widget_ {nullptr};

    ASTERIXConfigWidget* config_widget_ {nullptr};

    QCheckBox* debug_check_ {nullptr};
    QCheckBox* limit_ram_check_ {nullptr};
    QPushButton* create_mapping_stubs_button_ {nullptr};
    QPushButton* test_button_ {nullptr};
    QPushButton* import_button_ {nullptr};

    void updateParserList ();
    void createObjectParserWidget();
};

#endif // ASTERIXIMPORTERTASKWIDGET_H
