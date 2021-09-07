#pragma once

#include <QColor>
#include <QString>

#include <corelib_global.h>


namespace BusinessLayer {

/**
 * @brief Формат экспортируемого файла
 */
enum class ComicBookExportFileFormat {
    Pdf,
    Docx,
};

/**
 * @brief Опции экспорта
 */
struct CORE_LIBRARY_EXPORT ComicBookExportOptions {
    /**
     * @brief Путь к файлу
     */
    QString filePath;

    /**
     * @brief Формат файла
     */
    ComicBookExportFileFormat fileFormat = ComicBookExportFileFormat::Pdf;

    /**
     * @brief Идентификатор шаблона экспорта
     */
    QString templateId;

    /**
     * @brief Печатать титульную страницу
     */
    bool printTiltePage = true;

    /**
     * @brief Печатать ли блоки папок
     */
    bool printFolders = true;

    /**
     * @brief Использовать слова вместо цифр для заголовков страниц
     */
    bool useWordsInPageHeadings = false;

    /**
     * @brief Печатать ли комментарии по тексту
     */
    bool printInlineNotes = false;

    /**
     * @brief Печатать редакторские пометки
     */
    bool printReviewMarks = true;

    /**
     * @brief Водяной знак
     */
    QString watermark;
    /**
     * @brief Цвет водяного знака
     */
    QColor watermarkColor;


    //
    // Параметры самого документа
    //

    /**
     * @brief Верхний колонтитул
     */
    QString header;
    /**
     * @brief Нижний колонтитул
     */
    QString footer;
};

} // namespace BusinessLayer
