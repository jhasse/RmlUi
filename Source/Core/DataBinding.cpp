/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include "../../Include/RmlUi/Core/DataBinding.h"
#include "../../Include/RmlUi/Core/Element.h"

namespace Rml {
namespace Core {


DataViewText::DataViewText(Element* in_parent_element, const String& in_text, const size_t index_begin_search) : parent_element(in_parent_element->GetObserverPtr())
{
    text.reserve(in_text.size());

    bool success = true;

    size_t previous_close_brackets = 0;
    size_t begin_brackets = index_begin_search;
    while ((begin_brackets = in_text.find("{{", begin_brackets)) != String::npos)
    {
        text.insert(text.end(), in_text.begin() + previous_close_brackets, in_text.begin() + begin_brackets);

        const size_t begin_name = begin_brackets + 2;
        const size_t end_name = in_text.find("}}", begin_name);

        if (end_name == String::npos)
        {
            success = false;
            break;
        }

        DataEntry entry;
        entry.index = text.size();
        entry.name = (String)StringUtilities::StripWhitespace(StringView(in_text.data() + begin_name, in_text.data() + end_name));
        data_entries.push_back(std::move(entry));

        previous_close_brackets = end_name + 2;
        begin_brackets = previous_close_brackets;
    }

    if (data_entries.empty())
        success = false;

    if (success && previous_close_brackets < in_text.size())
        text.insert(text.end(), in_text.begin() + previous_close_brackets, in_text.end());

    if (!success)
    {
        text.clear();
        data_entries.clear();
    }
}

bool DataViewText::Update(const DataModel& model)
{
    bool entries_modified = false;

    for (DataEntry& entry : data_entries)
    {
        String value = model.GetValue(entry.name);

        if (entry.value != value)
        {
            entry.value = value;
            entries_modified = true;
        }
    }

    if (entries_modified)
    {
        if (parent_element)
        {
            String rml = CreateText();
            parent_element->SetInnerRML(rml);
        }
        else
        {
            Log::Message(Log::LT_WARNING, "Could not update data view text, parent element no longer valid. Was it destroyed?");
        }
    }

    return entries_modified;
}

String DataViewText::CreateText() const
{
    size_t reserve_size = text.size();

    for (const DataEntry& entry : data_entries)
        reserve_size += entry.value.size();

    String result;
    result.reserve(reserve_size);

    size_t previous_index = 0;
    for (const DataEntry& entry : data_entries)
    {
		result += text.substr(previous_index, entry.index - previous_index);
        result += entry.value;
        previous_index = entry.index;
    }

    if (previous_index < text.size())
        result += text.substr(previous_index);

    return result;
}


}
}
