#pragma once

#include "Command.h"
#include "CircularBuffer.h"

class Editor;

class InputController
{

private:

    Editor* m_editor;

    std::string m_commandBuffer;
    std::string m_repetitionBuffer;

    char m_findCharacter = '\0';
    bool m_searchedForward = true;

    int m_previousInput = 0;
    MODE m_previousMode = NORMAL_MODE;

    size_t m_lastSavedCommand = 0;

    CircularBuffer m_circularInputBuffer;

    std::pair<int, int> m_cursorPosOnVisualMode;

    void handleNormalModeInput(int input);
    void handleCommandModeInput(int input);
    void handleInsertModeInput(int input);
    void handleReplaceCharMode(int input);
    void handleVisualModes(int input);

    void handleCommandBufferInput();
    void handleDeleteCommands(int input);
    void handleFindCommand(int input);
    void handleDeleteToInsertCommands(int input);
    void handleGoCommands(int input);

    void clearRepetitionBuffer() { m_repetitionBuffer.clear(); }
    int repetitionCount();

    bool repeatedInput(int input) { return (input == m_previousInput) ;}
    void displayErrorMessage(const std::string& message);

public:

    InputController(Editor* editor);

    void handleInput();

    // Getters
    const std::string& commandBuffer() const { return m_commandBuffer; }
    const CircularBuffer& circularBuffer() const { return m_circularInputBuffer; }
    const std::pair<int, int>& initialVisualModeCursor() const { return m_cursorPosOnVisualMode; }
};
