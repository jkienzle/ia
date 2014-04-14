#include "Attack.h"

#include "Engine.h"

#include "GameTime.h"
#include "ItemWeapon.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Postmortem.h"
#include "Blood.h"
#include "Knockback.h"
#include "Log.h"
#include "Audio.h"

using namespace std;

void Attack::melee(Actor& attacker, const Weapon& wpn, Actor& defender) {
  MeleeAttackData data(attacker, wpn, defender, eng);

  printMeleeMsgAndPlaySfx(data, wpn);

  if(data.isEtherealDefenderMissed == false) {
    if(data.attackResult >= successSmall && data.isDefenderDodging == false) {
      const bool IS_DEFENDER_KILLED =
        data.curDefender->hit(data.dmg, wpn.getData().meleeDmgType, true);

      if(IS_DEFENDER_KILLED == false) {
        data.curDefender->getPropHandler().tryApplyPropFromWpn(wpn, true);
      }
      if(data.attackResult >= successNormal) {
        if(data.curDefender->getData().canBleed == true) {
          eng.gore->makeBlood(data.curDefender->pos);
        }
      }
      if(IS_DEFENDER_KILLED == false) {
        if(wpn.getData().meleeCausesKnockBack) {
          if(data.attackResult > successSmall) {
            eng.knockBack->tryKnockBack(
              *(data.curDefender), data.attacker->pos, false);
          }
        }
      }
      const ItemData& itemData = wpn.getData();
      if(
        itemData.itemWeight > itemWeight_light &&
        itemData.isIntrinsic == false) {
        Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes,
                data.curDefender->pos, NULL, SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd, eng);
      }
    }
  }

  if(data.curDefender == eng.player) {
    if(data.attackResult >= failSmall) {
      dynamic_cast<Monster*>(data.attacker)->isStealth = false;
    }
  } else {
    Monster* const monster = dynamic_cast<Monster*>(data.curDefender);
    monster->awareOfPlayerCounter_ = monster->getData().nrTurnsAwarePlayer;
  }
  eng.gameTime->actorDidAct();
}

void Attack::printMeleeMsgAndPlaySfx(const MeleeAttackData& data,
                                     const Weapon& wpn) {
  string otherName = "";

  if(data.isDefenderDodging) {
    //----- DEFENDER DODGES --------
    if(data.attacker == eng.player) {
      if(eng.player->isSeeingActor(*data.curDefender, NULL)) {
        otherName = data.curDefender->getNameThe();
      } else {
        otherName = "It ";
      }
      eng.log->addMsg(otherName + " dodges my attack.");
    } else {
      if(eng.player->isSeeingActor(*data.attacker, NULL)) {
        otherName = data.attacker->getNameThe();
      } else {
        otherName = "It";
      }
      eng.log->addMsg("I dodge an attack from " + otherName + ".", clrMsgGood);
    }
  } else if(data.attackResult <= failSmall) {
    //----- BAD AIMING --------
    if(data.attacker == eng.player) {
      if(data.attackResult == failSmall) {
        eng.log->addMsg("I barely miss!");
      } else if(data.attackResult == failNormal) {
        eng.log->addMsg("I miss.");
      } else if(data.attackResult == failBig) {
        eng.log->addMsg("I miss completely.");
      }
      Audio::play(wpn.getData().meleeMissSfx);
    } else {
      if(eng.player->isSeeingActor(*data.attacker, NULL)) {
        otherName = data.attacker->getNameThe();
      } else {
        otherName = "It";
      }
      if(data.attackResult == failSmall) {
        eng.log->addMsg(otherName + " barely misses me!", clrWhite, true);
      } else if(data.attackResult == failNormal) {
        eng.log->addMsg(otherName + " misses me.", clrWhite, true);
      } else if(data.attackResult == failBig) {
        eng.log->addMsg(otherName + " misses me completely.", clrWhite, true);
      }
    }
  } else {
    //----- AIM IS CORRECT -------
    if(data.isEtherealDefenderMissed) {
      //----- ATTACK MISSED DUE TO ETHEREAL TARGET --------
      if(data.attacker == eng.player) {
        if(eng.player->isSeeingActor(*data.curDefender, NULL)) {
          otherName = data.curDefender->getNameThe();
        } else {
          otherName = "It ";
        }
        eng.log->addMsg(
          "My attack passes right through " + otherName + "!");
      } else {
        if(eng.player->isSeeingActor(*data.attacker, NULL)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }
        eng.log->addMsg(
          "The attack of " + otherName + " passes right through me!",
          clrMsgGood);
      }
    } else {
      //----- ATTACK CONNECTS WITH DEFENDER --------
      //Determine the relative "size" of the hit
      MeleeHitSize hitSize = MeleeHitSize::small;
      const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
      if(MAX_DMG_ROLL >= 4) {
        if(data.dmgRoll > (MAX_DMG_ROLL * 5) / 6) {
          hitSize = MeleeHitSize::hard;
        } else if(data.dmgRoll >  MAX_DMG_ROLL / 2) {
          hitSize = MeleeHitSize::medium;
        }
      }

      //Punctuation depends on attack strength
      string dmgPunct = ".";
      switch(hitSize) {
        case MeleeHitSize::small:                     break;
        case MeleeHitSize::medium:  dmgPunct = "!";   break;
        case MeleeHitSize::hard:    dmgPunct = "!!!"; break;
      }

      if(data.attacker == eng.player) {
        const string wpnVerb = wpn.getData().meleeAttMsgs.player;

        if(eng.player->isSeeingActor(*data.curDefender, NULL)) {
          otherName = data.curDefender->getNameThe();
        } else {
          otherName = "it";
        }

        if(data.isIntrinsicAttack) {
          const string ATTACK_MOD_STR = data.isWeakAttack ? " feebly" : "";
          eng.log->addMsg(
            "I " + wpnVerb + " " + otherName + ATTACK_MOD_STR + dmgPunct,
            clrMsgGood);
        } else {
          const string ATTACK_MOD_STR =
            data.isWeakAttack  ? "feebly "    :
            data.isBackstab    ? "covertly "  : "";
          const SDL_Color clr =
            data.isBackstab ? clrBlueLgt : clrMsgGood;
          const string wpnName_a =
            eng.itemDataHandler->getItemRef(wpn, ItemRefType::a, true);
          eng.log->addMsg(
            "I " + wpnVerb + " " + otherName + " " + ATTACK_MOD_STR +
            "with " + wpnName_a + dmgPunct,
            clr);
        }
      } else {
        const string wpnVerb = wpn.getData().meleeAttMsgs.other;

        if(eng.player->isSeeingActor(*data.attacker, NULL)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }

        eng.log->addMsg(otherName + " " + wpnVerb + dmgPunct,
                        clrMsgBad, true);
      }

      SfxId hitSfx = SfxId::endOfSfxId;
      switch(hitSize) {
        case MeleeHitSize::small: {
          hitSfx = wpn.getData().meleeHitSmallSfx;
        } break;
        case MeleeHitSize::medium: {
          hitSfx = wpn.getData().meleeHitMediumSfx;
        } break;
        case MeleeHitSize::hard: {
          hitSfx = wpn.getData().meleeHitHardSfx;
        } break;
      }
      Audio::play(hitSfx);
    }
  }
}


