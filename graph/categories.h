#ifndef AGGPLOT_CATEGORIES_H
#define AGGPLOT_CATEGORIES_H

#include "strpp.h"
#include "units.h"
#include "utils.h"

struct category_item {
    double value;
    str text;
    category_item(double v, const char*t) : value(v), text(t) {}
};

class category_map : public ptr_list<category_item> {

    typedef category_item item;

public:
    class iterator : public label_iterator {
    public:
        iterator(const category_map& cat) : m_index(0), m_cat(cat) { }

        virtual bool next(double& val, const char*& text)
        {
            if (m_index >= m_cat.size())
                return false;

            item* it = m_cat[m_index];
            val = it->value;
            text = it->text.cstr();
            m_index++;
            return true;
        }

    private:
        unsigned m_index;
        const category_map& m_cat;
    };

public:
    void add_item(double v, const char* name) {
        add(new item(v, name));
    }
};

#endif
