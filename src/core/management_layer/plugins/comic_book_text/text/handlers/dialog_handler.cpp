#include "dialog_handler.h"

#include "../comic_book_text_edit.h"

#include <business_layer/templates/comic_book_template.h>

#include <QKeyEvent>
#include <QTextBlock>

using BusinessLayer::TextBlockStyle;
using BusinessLayer::TextParagraphType;
using Ui::ComicBookTextEdit;


namespace KeyProcessingLayer {

DialogHandler::DialogHandler(ComicBookTextEdit* _editor)
    : StandardKeyHandler(_editor)
{
}

void DialogHandler::handleEnter(QKeyEvent*)
{
    //
    // Получим необходимые значения
    //
    // ... курсор в текущем положении
    QTextCursor cursor = editor()->textCursor();
    // ... блок текста в котором находится курсор
    QTextBlock currentBlock = cursor.block();
    // ... текст блока
    QString currentBlockText = currentBlock.text().trimmed();
    // ... текст до курсора
    QString cursorBackwardText = currentBlockText.left(cursor.positionInBlock());
    // ... текст после курсора
    QString cursorForwardText = currentBlockText.mid(cursor.positionInBlock());


    //
    // Обработка
    //
    if (editor()->isCompleterVisible()) {
        //! Если открыт подстановщик

        //
        // Ни чего не делаем
        //
    } else {
        //! Подстановщик закрыт

        if (cursor.hasSelection()) {
            //! Есть выделение

            //
            // Удаляем всё, но оставляем стилем блока текущий
            //
            editor()->addParagraph(TextParagraphType::Dialogue);
        } else {
            //! Нет выделения

            if (cursorBackwardText.isEmpty() && cursorForwardText.isEmpty()) {
                //! Текст пуст

                //
                // Меняем стиль блока на описание действия
                //
                editor()->setCurrentParagraphType(changeForEnter(TextParagraphType::Dialogue));
            } else {
                //! Текст не пуст

                if (cursorBackwardText.isEmpty()) {
                    //! В начале блока

                    //
                    // Ни чего не делаем
                    //
                } else if (cursorForwardText.isEmpty()) {
                    //! В конце блока

                    //
                    // Перейдём к следующему блоку
                    //
                    editor()->moveCursor(QTextCursor::NextBlock);
                    editor()->addParagraph(jumpForEnter(TextParagraphType::Dialogue));
                } else {
                    //! Внутри блока

                    //
                    // Разрываем диалог блоком персонажа, вставляя его имя
                    //
                    {
                        //
                        // Найти име персонажа, кому принадлежит реплика
                        //
                        QString characterName;
                        {
                            QTextCursor cursor = editor()->textCursor();
                            QTextBlock cursorBlock = cursor.block();
                            while ((TextBlockStyle::forBlock(cursorBlock)
                                        != TextParagraphType::Character
                                    || TextBlockStyle::forBlock(cursorBlock)
                                        == TextParagraphType::Dialogue
                                    || TextBlockStyle::forBlock(cursorBlock)
                                        == TextParagraphType::PageSplitter)
                                   && !cursor.atStart()) {
                                cursor.movePosition(QTextCursor::PreviousBlock);
                                cursorBlock = cursor.block();
                            }

                            if (TextBlockStyle::forBlock(cursorBlock)
                                == TextParagraphType::Character) {
                                characterName = cursorBlock.text().simplified();
                            }
                        }

                        //
                        // Вставляем блок "герой" и добавляем имя
                        //
                        editor()->addParagraph(TextParagraphType::Character);
                        editor()->insertPlainText(characterName);

                        //
                        // Оставшийся текст форматируем, как "диалог"
                        //
                        editor()->addParagraph(TextParagraphType::Dialogue);
                    }
                }
            }
        }
    }
}

void DialogHandler::handleTab(QKeyEvent*)
{
    //
    // Получим необходимые значения
    //
    // ... курсор в текущем положении
    QTextCursor cursor = editor()->textCursor();
    // ... блок текста в котором находится курсор
    QTextBlock currentBlock = cursor.block();
    // ... текст до курсора
    QString cursorBackwardText = currentBlock.text().left(cursor.positionInBlock());
    // ... текст после курсора
    QString cursorForwardText = currentBlock.text().mid(cursor.positionInBlock());


    //
    // Обработка
    //
    if (editor()->isCompleterVisible()) {
        //! Если открыт подстановщик

        //
        // Ни чего не делаем
        //
    } else {
        //! Подстановщик закрыт

        if (cursor.hasSelection()) {
            //! Есть выделение

            //
            // Ни чего не делаем
            //
        } else {
            //! Нет выделения

            if (cursorBackwardText.isEmpty() && cursorForwardText.isEmpty()) {
                //! Текст пуст

                //
                // Меняем стиль на ремарку
                //
                editor()->setCurrentParagraphType(changeForTab(TextParagraphType::Dialogue));
            } else {
                //! Текст не пуст

                if (cursorBackwardText.isEmpty()) {
                    //! В начале блока

                    //
                    // Ни чего не делаем
                    //
                } else if (cursorForwardText.isEmpty()) {
                    //! В конце блока

                    //
                    // Вставляем блок ремарки
                    //
                    editor()->moveCursor(QTextCursor::NextBlock);
                    editor()->addParagraph(jumpForTab(TextParagraphType::Dialogue));
                } else {
                    //! Внутри блока

                    //
                    // Ничего не делаем
                    //
                }
            }
        }
    }
}

} // namespace KeyProcessingLayer
