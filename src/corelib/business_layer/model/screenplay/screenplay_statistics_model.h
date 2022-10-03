#pragma once

#include "../abstract_model.h"


namespace BusinessLayer {

class ScreenplayCastReport;
class ScreenplaySummaryReport;
class ScreenplayTextModel;

/**
 * @brief Модель статистики по сценарию
 */
class CORE_LIBRARY_EXPORT ScreenplayStatisticsModel : public AbstractModel
{
    Q_OBJECT

public:
    explicit ScreenplayStatisticsModel(QObject* _parent = nullptr);
    ~ScreenplayStatisticsModel() override;

    /**
     * @brief Модель текста сценария
     */
    ScreenplayTextModel* textModel() const;
    void setScreenplayTextModel(ScreenplayTextModel* _model);

    /**
     * @brief Перестроить отчёты
     */
    void updateReports();

    /**
     * @brief Суммарный отчёт по сценарию
     */
    const ScreenplaySummaryReport& summaryReport() const;

    /**
     * @brief Отчёт по персонажам
     */
    const ScreenplayCastReport& castReport() const;

protected:
    /**
     * @brief Реализация модели для работы с документами
     */
    /** @{ */
    void initDocument() override;
    void clearDocument() override;
    QByteArray toXml() const override;
    /** @} */

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace BusinessLayer
