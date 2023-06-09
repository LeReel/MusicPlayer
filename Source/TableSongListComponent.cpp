#include "TableSongListComponent.h"
#include "SceneComponent.h"

#pragma region SongTableElement
SongTableElement::SongTableElement(const juce::File& _associatedFile)
{
    associatedFile = _associatedFile;
    // Fills attributes map with associated file's metadata
    Utils::ReadMetadata(associatedFile, attributes);
    attributes["Favorite"] = attributes["Export"] = "";
}
#pragma endregion SongTableElement

#pragma region TableSongListComponent
TableSongListComponent::TableSongListComponent()
{
    columnsList = ATTRIBUTES_LIST;

    rowColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId);
    rowColour_interpolated = rowColour.interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId),
                                                        0.03f);
}

TableSongListComponent::~TableSongListComponent() = default;

void TableSongListComponent::cellClicked(int rowNumber, int columnId, const juce::MouseEvent&)
{
    //If not Favorite cell
    if (columnId != 4)
    {
        return;
    }

    SongTableElement* _songElement = dataList[rowNumber];
    if (!_songElement)
    {
        return;
    }
    _songElement->SwitchIsFavorite();

    if (SceneComponent* _sC = dynamic_cast<SceneComponent*>(componentOwner))
    {
        _sC->onFavoriteClicked(*_songElement);
    }

    table.updateContent();

    repaint();
    resized();
}

void TableSongListComponent::cellDoubleClicked(int rowNumber,
                                               int columnId,
                                               const juce::MouseEvent&)
{
    //If favorite
    if (columnId == 4)
    {
        return;
    }

    SetCurrentSelected(rowNumber);
    if (SceneComponent* _sC = dynamic_cast<SceneComponent*>(componentOwner))
    {
        _sC->onSongChose(GetCurrentSelected());
    }
    table.updateContent();
}

void TableSongListComponent::paintRowBackground(juce::Graphics& g,
                                                int rowNumber,
                                                int,
                                                int,
                                                bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::rebeccapurple);
    else if (rowNumber % 2)
        g.fillAll(rowColour_interpolated);
}

void TableSongListComponent::paintCell(juce::Graphics& g,
                                       int rowNumber,
                                       int columnId,
                                       int width,
                                       int height,
                                       bool)
{
    g.setColour(rowNumber == currentPlayingRow
                    ? juce::Colours::darkorange
                    : juce::Colours::whitesmoke);
    g.setFont(font);

    SongTableElement& _element = *dataList[rowNumber];

    const juce::String _attribute = _element.GetStringAttribute(columnsList[columnId - 1]);

    g.drawText(_attribute,
               2,
               0,
               width - 4,
               height,
               columnId == 4 || columnId == 5 //Favorite and Export columns
                   ? juce::Justification::centred
                   : juce::Justification::centredLeft,
               true);
}

juce::Component* TableSongListComponent::refreshComponentForCell(int,
                                                                 int,
                                                                 bool,
                                                                 Component* existingComponentToUpdate)
{
    jassert(existingComponentToUpdate == nullptr);
    return nullptr;
}

int TableSongListComponent::getColumnAutoSizeWidth(int columnId)
{
    int widest = 32;
    const int _numRows = getNumRows();

    for (int i = _numRows; --i >= 0;)
    {
        SongTableElement& _element = *dataList[i];

        juce::String _text = _element.GetStringAttribute(GetColumnNameAttribute(columnId - 1));

        widest = juce::jmax(widest, font.getStringWidth(_text));
    }

    return widest + 8;
}

SongTableElement& TableSongListComponent::GetCurrentSelected() const
{
    return *dataList[currentPlayingRow];
}

void TableSongListComponent::InitTableList(const juce::Array<juce::File>& _files)
{
    // Fills dataList with files
    LoadData(_files);

    const bool _isFirstInit = table.getHeader().getNumColumns(false) == 0;

    // Inits headers and colors
    if (_isFirstInit)
    {
        addAndMakeVisible(table);

        for (int i = 0, _columnId = 1; i < columnsList.size(); ++i, _columnId++)
        {
            const juce::String _columnString = columnsList[i];
            table.getHeader().addColumn(_columnString,
                                        _columnId,
                                        getColumnAutoSizeWidth(_columnId),
                                        50,
                                        400,
                                        juce::TableHeaderComponent::ColumnPropertyFlags::notSortable);
        }

        table.getHeader().setColour(juce::TableHeaderComponent::backgroundColourId, juce::Colours::rebeccapurple);
        table.setColour(juce::ListBox::outlineColourId, juce::Colours::darkorange);
    }
    table.setOutlineThickness(1);

    table.getHeader().setSortColumnId(1, true);
    table.setMultipleSelectionEnabled(false);

    resized();
}

void TableSongListComponent::SetCurrentSelected(const int rowNumber)
{
    currentPlayingRow = rowNumber;
}

void TableSongListComponent::SetCurrentSelected(const SongTableElement& _songElement)
{
    for (int i = 0; i < dataAmount; ++i)
    {
        if (dataList[i] == &_songElement)
        {
            currentPlayingRow = i;
            return;
        }
    }
}

void TableSongListComponent::LoadData(const juce::Array<juce::File>& _files)
{
    for (const auto& _file : _files)
    {
        if (_file == juce::File() || !_file.exists())
        {
            return;
        }
        auto* _element = new SongTableElement(_file);
        const juce::String _title = _element->GetStringAttribute("Title");

        bool _alreadyInList = false;
        // Checks if file is already in list
        for (SongTableElement* _data : dataList)
        {
            if (_data->GetStringAttribute("Title") == _title)
            {
                _alreadyInList = true;
                break;
            }
        }
        if (!_alreadyInList) dataList.add(_element);
    }
    dataAmount = dataList.size();
}

void TableSongListComponent::ChangeCell(const int _move,
                                        const bool _isLoopAll,
                                        const bool _isRandom)
{
    if (dataAmount == 0 || currentPlayingRow == -1) //No LoadedDatas or no CurrentPlaying yet selected
    {
        return;
    }
    if (_isRandom)
    {
        const int _maxRands = dataAmount, _alreadyPlayedSize = alreadyPlayedRandom.size();
        if (_alreadyPlayedSize == _maxRands)
        {
            alreadyPlayedRandom.clear();
        }
        int _rand = -1;
        bool _canExit = false;
        do
        {
            _rand = rand() % dataAmount;
            const bool _isPlaying = _rand == currentPlayingRow, _contains = alreadyPlayedRandom.contains(_rand);
            _canExit = !_isPlaying && !_contains;
        }
        while (!_canExit);

        alreadyPlayedRandom.add(_rand);
        currentPlayingRow = _rand;
    }
    // If _move is out of range 
    else if (currentPlayingRow + _move < 0 || currentPlayingRow + _move >= dataAmount)
    {
        if (!_isLoopAll)
        {
            return;
        }
        currentPlayingRow = currentPlayingRow + _move < 0
                                ? dataAmount - 1
                                : 0;
    }
    else
    {
        currentPlayingRow += _move;
    }

    if (SceneComponent* _sC = dynamic_cast<SceneComponent*>(componentOwner))
    {
        _sC->onSongChose(GetCurrentSelected());
    }
}

juce::String TableSongListComponent::GetColumnNameAttribute(int _columnId) const
{
    return columnsList[_columnId];
}
#pragma endregion TableSongListComponent
