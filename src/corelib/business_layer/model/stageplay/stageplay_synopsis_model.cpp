#include "stageplay_synopsis_model.h"


namespace BusinessLayer {

StageplaySynopsisModel::StageplaySynopsisModel(QObject* _parent)
    : SimpleTextModel(_parent)
{
    setName(tr("Synopsis"));
}

void StageplaySynopsisModel::setDocumentName(const QString& _name)
{
    Q_UNUSED(_name);
}

} // namespace BusinessLayer
