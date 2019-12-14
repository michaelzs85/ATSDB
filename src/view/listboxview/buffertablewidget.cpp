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


//#include <iostream>

#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "buffer.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "buffertablewidget.h"
#include "buffertablemodel.h"
#include "viewselection.h"
#include "listboxviewdatasource.h"

//using namespace Utils;

BufferTableWidget::BufferTableWidget(DBObject &object, ListBoxView& view, ListBoxViewDataSource& data_source,
                                     QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f), object_(object), view_(view), data_source_(data_source)
{
    setAutoFillBackground(true);

    QVBoxLayout *layout = new QVBoxLayout ();

    table_ = new QTableView (this);
    model_ = new BufferTableModel (this, object_, data_source_);
    table_->setModel(model_);

    connect (model_, SIGNAL(exportDoneSignal(bool)), this, SLOT(exportDoneSlot(bool)));

    layout->addWidget (table_);
    table_->show();

    setLayout (layout);

}

BufferTableWidget::~BufferTableWidget()
{
}



void BufferTableWidget::clear ()
{
    assert (model_);

    model_->clearData();
}

void BufferTableWidget::show (std::shared_ptr<Buffer> buffer) //, DBOVariableSet *variables, bool database_view
{
    assert (buffer);

    logdbg  << "BufferTableWidget: show: object " << object_.name() << " buffer size " << buffer->size()
            << " properties " << buffer->properties().size();
    assert (table_);
    assert (model_);

    model_->setData(buffer);
    table_->resizeColumnsToContents();

    logdbg  << " BufferTableWidget: show: end";
}

void BufferTableWidget::exportSlot(bool overwrite)
{
    loginf << "BufferTableWidget: exportSlot: object " << object_.name();

    QString file_name;
    if (overwrite)
    {
        file_name = QFileDialog::getSaveFileName(this, ("Save "+object_.name()+" as CSV").c_str(), "",
                                                 tr("Comma-separated values (*.csv);;All Files (*)"));
    }
    else
    {
        file_name = QFileDialog::getSaveFileName(this, ("Save "+object_.name()+" as CSV").c_str(), "",
                                                 tr("Comma-separated values (*.csv);;All Files (*)"), nullptr,
                                                 QFileDialog::DontConfirmOverwrite);
    }

    if (file_name.size())
    {
        loginf << "BufferTableWidget: exportSlot: export filename " << file_name.toStdString();
        assert (model_);
        model_->saveAsCSV(file_name.toStdString(), overwrite);
    }
    else
    {
        emit exportDoneSignal (true);
    }
}

void BufferTableWidget::exportDoneSlot (bool cancelled)
{
    emit exportDoneSignal (cancelled);
}

void BufferTableWidget::showOnlySelectedSlot (bool value)
{
    loginf << "BufferTableWidget: showOnlySelectedSlot: " << value;

    assert (model_);
    model_->showOnlySelected(value);
    assert (table_);
    table_->resizeColumnsToContents();
}

void BufferTableWidget::usePresentationSlot (bool use_presentation)
{
    assert (model_);
    model_->usePresentation(use_presentation);
    assert (table_);
    table_->resizeColumnsToContents();
}

void BufferTableWidget::showAssociationsSlot (bool value)
{
    assert (model_);
    model_->showAssociations(value);
    assert (table_);
    table_->resizeColumnsToContents();
}


void BufferTableWidget::resetModel()
{
    assert (model_);
    model_->reset();
}

void BufferTableWidget::updateToSelection ()
{
    assert (model_);
    model_->updateToSelection();
    assert (table_);
    table_->resizeColumnsToContents();
}

ListBoxView &BufferTableWidget::view() const
{
    return view_;
}

//void BufferTableWidget::itemChanged (QTableWidgetItem *item)
//{
//    if (selection_checkboxes_.find (item) != selection_checkboxes_.end())
//    {
//        unsigned int id = selection_checkboxes_[item];
//        bool checked = item->checkState() == Qt::Checked;
//        logdbg  << "BufferTableWidget: itemChanged: id " << id << " checked " << checked;

//        if (checked) // add
//        {
//            ViewSelectionEntries entries;
//            entries.push_back(ViewSelectionEntry (ViewSelectionId(type_, id), ViewSelectionEntry::TYPE_BILLBOARD));
//            ViewSelection::getInstance().addSelection(entries);
//        }
//        else // remove
//        {
//            ViewSelectionEntries selection_entries = ViewSelection::getInstance().getEntries();
//            ViewSelectionEntries::iterator it;

//            bool found=false;
//            for (it = selection_entries.begin(); it != selection_entries.end(); it++)
//            {
//                if (it->id_.first == type_ && it->id_.second == id)
//                {
//                    found = true;
//                    selection_entries.erase (it);
//                    break;
//                }
//            }

//            if (found)
//            {
//                logdbg  << "BufferTableWidget: itemChanged: unselecting type " << type_ << " id " << id;
//                ViewSelection::getInstance().clearSelection();
//                ViewSelection::getInstance().setSelection(selection_entries);
//            }
//            else
//            {
//                logwrn  << "BufferTableWidget: itemChanged: unselect failed for type " << type_ << " id " << id;
//            }
//        }
//    }
//    else
//        logerr << "BufferTableWidget: itemChanged: unknown table item";
//}

//void BufferTableWidget::keyPressEvent ( QKeyEvent * event )
//{
//    logdbg  << "BufferTableWidget: keyPressEvent: got keypressed";

//    assert (table_);

    //TODO
//    if (event->modifiers()  & Qt::ControlModifier)
//    {
//        if (event->key() == Qt::Key_C)
//        {
//            QAbstractItemModel *abmodel = table_->model();
//            QItemSelectionModel * model = table_->selectionModel();
//            QModelIndexList list = model->selectedIndexes();

//            qSort(list);

//            if(list.size() < 1)
//                return;

//            int min_col=0, max_col=0, min_row=0, max_row=0;

//            for(int i = 0; i < list.size(); i++)
//            {
//                QModelIndex index = list.at(i);

//                int row = index.row();
//                int col = index.column();

//                if (i==0)
//                {
//                    min_col=col;
//                    max_col=col;
//                    min_row=row;
//                    max_row=row;
//                }

//                if (row < min_row)
//                    min_row=row;
//                if (row > max_row)
//                    max_row=row;

//                if (col < min_col)
//                    min_col=col;
//                if (col > max_col)
//                    max_col=col;
//            }

//            int rows = max_row-min_row+1;
//            int cols = max_col-min_col+1;

//            if (rows < 1)
//            {
//                logwrn  << "BufferTableWidget: keyPressEvent: too few rows " << rows;
//                return;
//            }
//            logdbg  << "BufferTableWidget: keyPressEvent: rows " << rows;

//            if (cols < 1)
//            {
//                logwrn  << "BufferTableWidget: keyPressEvent: too few cols " << cols;
//                return;
//            }
//            logdbg  << "BufferTableWidget: keyPressEvent: cols " << cols;

//            std::vector < std::vector <std::string> > table_strings (rows,std::vector<std::string> (cols));

//            for(int i = 0; i < list.size(); i++)
//            {
//                QModelIndex index = list.at(i);
//                QVariant data = abmodel->data(index);
//                QString text = data.toString();

//                QTableWidgetItem *item = table_->item (index.row(), index.column());
//                if( item->checkState() == Qt::Checked )
//                    text = "X";

//                table_strings.at(index.row()-min_row).at(index.column()-min_col) = "\""+text.toStdString()+"\"";
//            }

//            QString copy_table;
//            for(int i = 0; i < (int)table_strings.size(); i++)
//            {
//                std::vector <std::string> &row_strings = table_strings.at(i);
//                for(int j = 0; j < (int)row_strings.size(); j++)
//                {
//                    if (j != 0)
//                        copy_table.append('\t');
//                    copy_table.append(row_strings.at(j).c_str());
//                }
//                copy_table.append('\n');
//            }

//            // make col indexes
//            std::set<unsigned int> col_indexes;
//            for (int i = min_col; i <= max_col; i++)
//                col_indexes.insert(i);

//            //set header
//            std::set<unsigned int>::iterator it;
//            QString header_string;
//            for (it = col_indexes.begin(); it != col_indexes.end(); it++)
//            {
//                if (it != col_indexes.begin())
//                    header_string.append('\t');
//                assert (*it < (unsigned int) header_list_.size());
//                header_string.append(header_list_.at(*it));
//            }
//            header_string.append('\n');

//            QClipboard *clipboard = QApplication::clipboard();
//            clipboard->setText(header_string+copy_table);
//        }
//    }
//}
