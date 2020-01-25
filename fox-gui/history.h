#ifndef FOXGUI_HISTORY_H
#define FOXGUI_HISTORY_H

#include <string>
#include <vector>

class history {
    using size_type = std::vector<std::string>::size_type;
public:
    history(): m_index(0) {}

    void add(const char* line)
    {
        m_lines.push_back(std::string{line});
        m_index = 0;
    }

    void remove_last() { m_lines.pop_back(); }

    bool is_first() const { return m_index == 0; }

    const char* previous()
    {
        if (m_index < m_lines.size()) m_index++;
        const char* ln = line(m_index);
        return ln;
    }

    const char* next()
    {
        if (m_index > 0) m_index--;
        const char* ln = line(m_index);
        return ln;
    }

    const char* line(int j)
    {
        size_type sz = m_lines.size();
        int index = int(sz) - j;

        if (index >= 0 && index < int(sz))
        {
            return m_lines[index].c_str();
        }
        return 0;
    }

private:
    std::vector<std::string> m_lines;
    size_type m_index;
};

#endif
