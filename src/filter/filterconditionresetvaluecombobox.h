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

#ifndef FILTERCONDITIONRESETVALUECOMBOBOX_H_
#define FILTERCONDITIONRESETVALUECOMBOBOX_H_

#include <QtWidgets/QComboBox>
#include <QtCore/QList>

class FilterConditionResetValueComboBox : public QComboBox
{
protected:
  static QList<QString> stringsList_;
public:
  FilterConditionResetValueComboBox ()
  {
    if (stringsList_.size() == 0)
    {
      stringsList_.append("MIN");
      stringsList_.append("MAX");
      stringsList_.append("value");
    }

    /* Populate the comboBox */
    addItems(stringsList_);
  }
  virtual ~FilterConditionResetValueComboBox () {}

};

QList<QString> FilterConditionResetValueComboBox::stringsList_;


#endif /* FILTERCONDITIONRESETVALUECOMBOBOX_H_ */
