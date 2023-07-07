#pragma once

#include <corelib_global.h>

class QAbstractItemModel;

namespace BusinessLayer {
class SimpleTextModel;
}


/**
 * @brief Вспомогательные методы для моделей данных
 */
class CORE_LIBRARY_EXPORT ModelHelper
{
public:
    /**
     * @brief Получить количество всех строк в таблице, включая вложенные
     */
    static int recursiveRowCount(QAbstractItemModel* _model);

    /**
     * @brief Инициилизировать модель титульной страницы
     */
    static void initTitlePageModel(BusinessLayer::SimpleTextModel* _model);

    /**
     * @brief Сбросить модель титульной страницы на дефолтную
     */
    static void resetTitlePageModel(BusinessLayer::SimpleTextModel* _model);
};
