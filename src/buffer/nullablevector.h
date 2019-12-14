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

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

#include <memory>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <array>
#include <set>
#include <map>

#include <tbb/tbb.h>

#include <QtCore/QDateTime>

#include "stringconv.h"
#include "buffer.h"
#include "property.h"

const bool BUFFER_PEDANTIC_CHECKING=false;

/**
 * @brief Template List of fixed-size arrays to be used in Buffer classes.
 *
 * Was written for easy management of arrays of different data types.
 */
template <class T>
class NullableVector
{
    friend class Buffer;

public:
    /// @brief Destructor
    virtual ~NullableVector () {}

    /// @brief Sets all elements to false
    void clear();

    /// @brief Returns const reference to a specific value
    const T get (size_t index);

    /// @brief Returns string of a specific value
    const std::string getAsString (size_t index);

    /// @brief Sets specific value
    void set (size_t index, T value);
    void setFromFormat (size_t index, const std::string& format, const std::string& value_str);

    /// @brief Appends specific value
    void append (size_t index, T value);
    void appendFromFormat (size_t index, const std::string& format, const std::string& value_str);

    /// @brief Sets specific element to Null value
    void setNull(size_t index);

    NullableVector<T>& operator*=(double factor);

    std::set<T> distinctValues (size_t index=0);

    std::map<T, std::vector<size_t>> distinctValuesWithIndexes (size_t from_index, size_t to_index);
    std::map<T, std::vector<size_t>> distinctValuesWithIndexes (const std::vector<size_t>& indexes);
    std::vector<size_t> nullValueIndexes (size_t from_index, size_t to_index);
    std::vector<size_t> nullValueIndexes (const std::vector<size_t>& indexes);

    void convertToStandardFormat(const std::string& from_format);

    size_t size();

    /// @brief Checks if specific element is Null
    bool isNull(size_t index);

    void checkNotNull ();

    std::string propertyName () const { return property_.name()+"("+property_.dataTypeString()+")"; }

private:
    Property property_;
    Buffer& buffer_;
    /// Data container
    std::vector<T> data_;
    // Null flags container
    std::vector <bool> null_flags_;

    /// @brief Sets specific element to not Null value
    void unsetNull (size_t index);

    void resizeDataTo (size_t size);
    void resizeNullTo (size_t size);
    void addData (NullableVector<T>& other);
    void copyData (NullableVector<T>& other);
    void cutToSize (size_t size);

    /// @brief Constructor, only for friend Buffer
    NullableVector (Property& property, Buffer& buffer);

};


template <class T> NullableVector<T>::NullableVector (Property& property, Buffer& buffer)
    : property_(property), buffer_(buffer) {}

template <class T> void NullableVector<T>::clear()
{
    logdbg << "ArrayListTemplate " << property_.name() << ": clear";
    std::fill (data_.begin(),data_.end(), T());
    std::fill (null_flags_.begin(), null_flags_.end(), true);
}

template <class T> const T NullableVector<T>::get (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": get: index " << index;
    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
        assert (index < data_.size());
        assert (index < data_.size());
    }

    if (isNull(index))
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            logerr << "ArrayListTemplate " << property_.name() << ": get: index " << index << " is null";
            assert (false);
        }

        throw std::runtime_error ("ArrayListTemplate: get of Null value "+std::to_string(index));
    }

    return data_.at(index);
}

template <class T> const std::string NullableVector<T>::getAsString (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": getAsString";
    return Utils::String::getValueString (get(index));
}

template <class T> void NullableVector<T>::set (size_t index, T value)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": set: index " << index << " value '" << value << "'";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= data_.size()) // allocate new stuff, fill all new with not null
    {
        if (index != data_.size()) // some where left out
            resizeNullTo(index+1);

        resizeDataTo (index+1);
    }

    if (BUFFER_PEDANTIC_CHECKING)
        assert (index < data_.size());

    data_.at(index) = value;
    unsetNull(index);

    //logdbg << "ArrayListTemplate: set: size " << size_ << " max_size " << max_size_;
}

template <class T> void NullableVector<T>::setFromFormat (size_t index, const std::string& format,
                                                          const std::string& value_str)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": setFromFormat";
    T value;

    if (format == "octal")
    {
        value = std::stoi(value_str, 0, 8);
    }
    else if (format == "hexadecimal")
    {
        value = std::stoi(value_str, 0, 16);
    }
    else if (format == "epoch_tod_ms")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "epoch_tod_s")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(1000*std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else
    {
        logerr << "ArrayListTemplate: setFromFormat: unknown format '" << format << "'";
        assert (false);
    }

    set (index, value);
}

template <class T> void NullableVector<T>::append (size_t index, T value)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": append: index " << index << " value '" << value << "'";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= data_.size()) // allocate new stuff, fill all new with not null
    {
        if (index != data_.size()) // some where left out
            resizeNullTo(index+1);

        resizeDataTo (index+1);
    }

    if (BUFFER_PEDANTIC_CHECKING)
        assert (index < data_.size());

    data_.at(index) += value;
    unsetNull(index);

    //logdbg << "ArrayListTemplate: set: size " << size_ << " max_size " << max_size_;
}

template <class T> void NullableVector<T>::appendFromFormat (size_t index, const std::string& format,
                                                             const std::string& value_str)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": appendFromFormat";
    T value;

    if (format == "octal")
    {
        value = std::stoi(value_str, 0, 8);
    }
    else if (format == "hexadecimal")
    {
        value = std::stoi(value_str, 0, 16);
    }
    else if (format == "epoch_tod_ms")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "epoch_tod_s")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(1000*std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else
    {
        logerr << "ArrayListTemplate: setFromFormat: unknown format '" << format << "'";
        assert (false);
    }

    append (index, value);
}

template <class T> void NullableVector<T>::setNull(size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": setNull: index " << index;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= null_flags_.size()) // null flags to small
        resizeNullTo (index+1);

    if (BUFFER_PEDANTIC_CHECKING)
        assert (index < null_flags_.size());

    null_flags_.at(index) = true;
}

/// @brief Checks if specific element is Null
template <class T> bool NullableVector<T>::isNull(size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": isNull: index " << index;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
        assert (index < buffer_.data_size_);
    }

    if (index < null_flags_.size()) // if stored, return value
        return null_flags_.at(index);

    // null not stored, so all set are not null

    if (index >= data_.size()) // not yet set
        return true;

    // must be set
    return false;
}

template <class T> void NullableVector<T>::resizeDataTo (size_t size)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": resizeDataTo: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (data_.size() < size); // only to be called if needed
    }

    data_.resize(size, T());

    if (buffer_.data_size_ < data_.size()) // set new data size
        buffer_.data_size_ = data_.size();
}

template <class T> void NullableVector<T>::resizeNullTo (size_t size)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": resizeNullTo: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
        assert (null_flags_.size() <= buffer_.data_size_);

    if (data_.size() > null_flags_.size()) // data was set w/o null, adjust & fill with set values
        null_flags_.resize(data_.size(), false);

    if (null_flags_.size() < size) // adjust to new size, fill with null values
        null_flags_.resize(size, true);

    if (buffer_.data_size_ < null_flags_.size()) // set new data size
        buffer_.data_size_ = null_flags_.size();

    if (BUFFER_PEDANTIC_CHECKING)
        assert (size == null_flags_.size());
}

template <class T> void NullableVector<T>::addData (NullableVector<T>& other)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": addData";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

    if (!other.data_.size() && other.null_flags_.size()) // if other has null flags set, need to fill my nulls
    {
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 1: other no data resizing null";
        resizeNullTo (buffer_.data_size_);
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 1: inserting null";
        null_flags_.insert(null_flags_.end(), other.null_flags_.begin(), other.null_flags_.end());
        goto DONE;
    }

    if (other.data_.size() && !other.null_flags_.size()) // if other has everything set
    {
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: other has everything set";

        if (data_.size() < buffer_.data_size_) // need to size data up
        {
            logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: data not full, setting null";
            resizeNullTo (buffer_.data_size_);

            logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: resizing data";
            resizeDataTo (buffer_.data_size_);
        }

        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: inserting data";
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        goto DONE;
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: mixture, both have data & nulls";

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: resizing null to " << buffer_.data_size_;
    resizeNullTo (buffer_.data_size_);
    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: inserting nulls";
    null_flags_.insert(null_flags_.end(), other.null_flags_.begin(), other.null_flags_.end());

    if (data_.size() < buffer_.data_size_) // need to size data up
    {
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: resizing data";
        resizeDataTo (buffer_.data_size_);
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: inserting data";
    data_.insert(data_.end(), other.data_.begin(), other.data_.end());

DONE:
    // size is adjusted in Buffer::seizeBuffer

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: end";
}

template <class T> void NullableVector<T>::copyData (NullableVector<T>& other)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": copyData";

    data_ = other.data_;
    null_flags_ = other.null_flags_;

    // is only done for new buffers in Buffer::getPartialCopy, so no size-too-big isse

    if (buffer_.data_size_ < data_.size())
        buffer_.data_size_ = data_.size();

    if (buffer_.data_size_ < null_flags_.size())
        buffer_.data_size_ = null_flags_.size();

    logdbg << "ArrayListTemplate " << property_.name() << ": copyData: end";
}

template <class T> NullableVector<T>& NullableVector<T>::operator*=(double factor)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": operator*=";

    size_t data_size = data_.size();

    tbb::parallel_for( size_t(0), data_size, [&] (size_t cnt)
    {
        if (!isNull(cnt))
        {
            data_.at(cnt) *= factor;
        }
    });

//    for (auto &data_it : data_)
//        data_it *= factor;

    return *this;
}

template <class T> std::set<T> NullableVector<T>::distinctValues (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValues";

    std::set<T> values;

    T value;

    for (; index < data_.size(); ++index)
    {
        if (!isNull(index)) // not for null
        {
            value = data_.at(index);
            if (values.count(value) == 0)
                values.insert(value);
        }
    }

    return values;
}

template <class T> std::map<T, std::vector<size_t>> NullableVector<T>::distinctValuesWithIndexes (size_t from_index,
                                                                                                  size_t to_index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValuesWithIndexes";

    std::map<T, std::vector<size_t>> values;

    assert (from_index < to_index);

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (to_index);
        assert (from_index < buffer_.data_size_);
        assert (to_index < buffer_.data_size_);
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

    if (from_index+1 > data_.size()) // no data
        return values;

    for (size_t index = from_index; index <= to_index; ++index)
    {
        if (!isNull(index)) // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert (index < data_.size());

            values[data_.at(index)].push_back(index);
        }
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValuesWithIndexes: done with " << values.size();
    return values;
}

template <class T> std::map<T, std::vector<size_t>> NullableVector<T>::distinctValuesWithIndexes (
        const std::vector<size_t>& indexes)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValuesWithIndexes";

    std::map<T, std::vector<size_t>> values;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

    for (auto index : indexes)
    {
        if (!isNull(index)) // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert (index < data_.size());

            values[data_.at(index)].push_back(index);
        }
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValuesWithIndexes: done with " << values.size();
    return values;
}

template <class T> std::vector<size_t>  NullableVector<T>::nullValueIndexes (size_t from_index,
                                                                             size_t to_index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": nullValueIndexes";

    std::vector<size_t> indexes;

    assert (from_index < to_index);

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (to_index);
        assert (from_index < buffer_.data_size_);
        assert (to_index < buffer_.data_size_);
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

//    if (from_index+1 >= data_.size()) // no data
//        return indexes;

    for (size_t index = from_index; index <= to_index; ++index)
    {
        if (isNull(index)) // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert (index < data_.size());

            indexes.push_back(index);
        }
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": nullValueIndexes: done with " << indexes.size();
    return indexes;
}

template <class T> std::vector<size_t>  NullableVector<T>::nullValueIndexes (const std::vector<size_t>& indexes)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": nullValueIndexes";

    std::vector<size_t> ret_indexes;

    for (auto index : indexes)
    {
        if (isNull(index)) // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert (index < data_.size());

            ret_indexes.push_back(index);
        }
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": nullValueIndexes: done with " << ret_indexes.size();
    return ret_indexes;
}

template <class T> void NullableVector<T>::convertToStandardFormat(const std::string& from_format)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": convertToStandardFormat";

    static_assert (std::is_integral<T>::value, "only defined for integer types");

    //std::string value_str;
    //T value;

    if (from_format != "octal")
    {
        logerr << "ArrayListTemplate: convertToStandardFormat: unknown format '" << from_format << "'";
        assert (false);
    }

    size_t data_size = data_.size();

    tbb::parallel_for( size_t(0), data_size, [&] (size_t cnt)
    {
        if (!isNull(cnt))
        {
            //value_str = std::to_string(data_.at(cnt));
            data_.at(cnt) = std::stoi(std::to_string(data_.at(cnt)), 0, 8);
        }
    });


    //    for (size_t cnt=0; cnt < data_size; cnt++)
    //    {
    //        if (isNull(cnt))
    //            continue;

    //        value_str = std::to_string(data_.at(cnt));

    //        if (from_format == "octal")
    //        {
    //            data_.at(cnt) = std::stoi(value_str, 0, 8);
    //        }
    //        else
    //        {
    //            logerr << "ArrayListTemplate: convertToStandardFormat: unknown format '" << from_format << "'";
    //            assert (false);
    //        }
    //    }
}

template <class T> size_t NullableVector<T>::size() { return data_.size(); }

template <class T> void NullableVector<T>::cutToSize (size_t size)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": cutToSize: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
    }

    while (null_flags_.size() > size)
        null_flags_.pop_back();

    while (data_.size() > size)
        data_.pop_back();

    // size set in Buffer::cutToSize
}

template <class T> void NullableVector<T>::checkNotNull ()
{
    logdbg << "ArrayListTemplate " << property_.name() << ": checkNotNull";

    for (size_t cnt=0; cnt < null_flags_.size(); cnt++)
    {
        if (null_flags_.at(cnt))
        {
            logerr << "cnt " << cnt << " null";
            assert (false);
        }
    }
}

// private stuff

/// @brief Sets specific element to not Null value
template <class T> void NullableVector<T>::unsetNull (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": unsetNull";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (null_flags_.size() <= buffer_.data_size_);
        assert (index < buffer_.data_size_);
        assert (index < data_.size());
    }

    if (index < null_flags_.size()) // if was already set
        null_flags_.at(index) = false;
}

template <>
NullableVector<bool>& NullableVector<bool>::operator*=(double factor);

template <>
void NullableVector<bool>::append (size_t index, bool value);

template <>
void NullableVector<std::string>::append (size_t index, std::string value);




#endif /* ARRAYLIST_H_ */
