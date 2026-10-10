#pragma once

#include "Commands/ICommand.h"
#include <vector>
#include <memory>

namespace Avero::Commands
{
    class CommandManager
    {
    public:
        CommandManager() = default;

        void Execute(std::shared_ptr<ICommand> command)
        {
            command->Execute();
            m_undoStack.push_back(command);
            m_redoStack.clear();
        }

        void Undo()
        {
            if (m_undoStack.empty()) return;

            auto command = m_undoStack.back();
            m_undoStack.pop_back();
            command->Undo();
            m_redoStack.push_back(command);
        }

        void Redo()
        {
            if (m_redoStack.empty()) return;

            auto command = m_redoStack.back();
            m_redoStack.pop_back();
            command->Execute();
            m_undoStack.push_back(command);
        }

        bool CanUndo() const { return !m_undoStack.empty(); }
        bool CanRedo() const { return !m_redoStack.empty(); }

        void ClearHistory()
        {
            m_undoStack.clear();
            m_redoStack.clear();
        }

    private:
        std::vector<std::shared_ptr<ICommand>> m_undoStack;
        std::vector<std::shared_ptr<ICommand>> m_redoStack;
    };
}