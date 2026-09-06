#include "spellaction.h"
#include "game.h"
#include "i18n.h"

void SpellAction::lookAtTarget()
{
  if (targetType != NoTarget)
    character->lookTo(getTargetPosition());
}

int SpellAction::getApCost() const
{
  return spell.getActionPointCost(character);
}

bool SpellAction::trigger()
{
  auto* i18n = I18n::get();
  auto* level = LevelTask::get();
  bool  isInCombat = level && level->isInCombat(character);

  if (!isInCombat || getApCost() <= character->getActionPoints())
  {
    QJSValue result;

    lookAtTarget();
    switch (targetType)
    {
    case ObjectTarget:
      result = spell.useOn(character, target);
      break ;
    case PositionTarget:
      result = spell.useAt(character, targetPosition.x(), targetPosition.y());
      break ;
    case NoTarget:
      result = spell.use(character);
      break ;
    }
    return triggerAnimation(result);
  }
  else
    emit level->displayConsoleMessage(i18n->t("messages.not-enough-ap"));
  return false;
}

void SpellAction::performAction()
{
  bool success = false;
  QJSValueList params;

  if (!callback.isCallable())
    callback = getDefaultCallback();
  params << character->asJSValue();
  switch (targetType)
  {
  case ObjectTarget:
    params << target->asJSValue();
    break ;
  case PositionTarget:
    params << targetPosition.x() << targetPosition.y();
    break ;
  case NoTarget:
    break ;
  }
  success = Game::get()->scriptCall(callback, params, "SpellAction::performAction").toBool();
  character->useActionPoints(getApCost(), "spellcasting");
  state = success ? Done : Interrupted;
}

QJSValue SpellAction::getDefaultCallback()
{
  return spell.triggerUse;
}
