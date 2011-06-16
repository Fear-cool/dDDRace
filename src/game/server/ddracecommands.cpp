/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "gamecontext.h"
#include <engine/shared/config.h>
#include <engine/server/server.h>
#include <game/server/teams.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/version.h>
#include <game/generated/nethash.cpp>
#if defined(CONF_SQL)
	#include <game/server/score/sql_score.h>
#endif

void CGameContext::ConGoLeft(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientID, pResult->GetVictim(), -1, 0);
}

void CGameContext::ConGoRight(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientID, pResult->GetVictim(), 1, 0);
}

void CGameContext::ConGoDown(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientID, pResult->GetVictim(), 0, 1);
}

void CGameContext::ConGoUp(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientID, pResult->GetVictim(), 0, -1);
}

void CGameContext::ConMove(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientID, pResult->GetVictim(), pResult->GetInteger(0), pResult->GetInteger(1));
}

void CGameContext::ConMoveRaw(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MoveCharacter(ClientID, pResult->GetVictim(), pResult->GetInteger(0), pResult->GetInteger(1), true);
}

void CGameContext::MoveCharacter(int ClientID, int Victim, int X, int Y, bool Raw)
{
	CCharacter* pChr = GetPlayerChar(Victim);

	if(!pChr)
		return;

	pChr->Core()->m_Pos.x += ((Raw) ? 1 : 32) * X;
	pChr->Core()->m_Pos.y += ((Raw) ? 1 : 32) * Y;
	pChr->m_DDRaceState = DDRACE_CHEAT;
}

void CGameContext::ConSetlvl3(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = 3;
		pServ->SetRconLevel(Victim, 3);
	}
}

void CGameContext::ConSetlvl2(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = 2;
		pServ->SetRconLevel(Victim, 2);
	}
}

void CGameContext::ConSetlvl1(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = 1;
		pServ->SetRconLevel(Victim, 1);
	}
}

void CGameContext::ConLogOut(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();

	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = IConsole::CONSOLELEVEL_USER;
		pServ->SetRconLevel(Victim, IConsole::CONSOLELEVEL_USER);
		if (g_Config.m_SvRconScore)
			pSelf->m_apPlayers[Victim]->m_Score = 0;
		pSelf->m_apPlayers[Victim]->m_IsMember = false;
		pSelf->m_apPlayers[Victim]->m_IsLoggedIn = false;
	}
}

void CGameContext::ConKillPlayer(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->KillCharacter(WEAPON_GAME);
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "%s was killed by %s", pSelf->Server()->ClientName(Victim), pSelf->Server()->ClientName(ClientID));
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
}

void CGameContext::ConNinja(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), WEAPON_NINJA, false);
}

void CGameContext::ConSuper(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(pChr && !pChr->m_Super)
	{
		pChr->m_Super = true;
		pChr->UnFreeze();
		pChr->m_TeamBeforeSuper = pChr->Team();
		pChr->Teams()->SetCharacterTeam(Victim, TEAM_SUPER);
		pChr->m_DDRaceState = DDRACE_CHEAT;
	}
}

void CGameContext::ConUnSuper(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CCharacter* pChr = pSelf->GetPlayerChar(pResult->GetVictim());
	if(pChr && pChr->m_Super)
	{
		pChr->m_Super = false;
		pChr->Teams()->SetForceCharacterTeam(pResult->GetVictim(), pChr->m_TeamBeforeSuper);
	}
}

void CGameContext::ConShotgun(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), WEAPON_SHOTGUN, false);
}

void CGameContext::ConGrenade(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), WEAPON_GRENADE, false);
}

void CGameContext::ConRifle(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), WEAPON_RIFLE, false);
}

void CGameContext::ConWeapons(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), -1, false);
}

void CGameContext::ConUnShotgun(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), WEAPON_SHOTGUN, true);
}

void CGameContext::ConUnGrenade(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), WEAPON_GRENADE, true);
}

void CGameContext::ConUnRifle(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), WEAPON_RIFLE, true);
}

void CGameContext::ConUnWeapons(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), -1, true);
}

void CGameContext::ConAddWeapon(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), pResult->GetInteger(0), false);
}

void CGameContext::ConRemoveWeapon(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, ClientID, pResult->GetVictim(), pResult->GetInteger(0), true);
}

void CGameContext::ModifyWeapons(IConsole::IResult *pResult, int ClientID, int Victim, int Weapon, bool Remove)
{
	if(clamp(Weapon, -1, NUM_WEAPONS - 1) != Weapon)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "invalid weapon id");
		return;
	}

	CCharacter* pChr = GetPlayerChar(Victim);
	if(!pChr)
		return;

	if(Weapon == -1)
	{
		if(Remove && (pChr->GetActiveWeapon() == WEAPON_SHOTGUN || pChr->GetActiveWeapon() == WEAPON_GRENADE || pChr->GetActiveWeapon() == WEAPON_RIFLE))
			pChr->SetActiveWeapon(WEAPON_GUN);

		if(Remove)
		{
			pChr->SetWeaponGot(WEAPON_SHOTGUN, false);
			pChr->SetWeaponGot(WEAPON_GRENADE, false);
			pChr->SetWeaponGot(WEAPON_RIFLE, false);
		}
		else
			pChr->GiveAllWeapons();
	}
	else if(Weapon != WEAPON_NINJA)
	{
		if(Remove && pChr->GetActiveWeapon() == Weapon)
			pChr->SetActiveWeapon(WEAPON_GUN);

		if(Remove)
			pChr->SetWeaponGot(Weapon, false);
		else
			pChr->GiveWeapon(Weapon, -1);
	}
	else
	{
		if(Remove)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "you can't remove ninja");
			return;
		}

		pChr->GiveNinja();
	}

	pChr->m_DDRaceState =	DDRACE_CHEAT;
}

void CGameContext::ConTeleport(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	int TeleTo = clamp(pResult->GetInteger(0), 0, (int)MAX_CLIENTS-1);
	if(pSelf->m_apPlayers[TeleTo])
	{
		{
			CCharacter* pChr = pSelf->GetPlayerChar(Victim);
			if(pChr)
			{
				pChr->Core()->m_Pos = pSelf->m_apPlayers[TeleTo]->m_ViewPos;
				pChr->m_DDRaceState = DDRACE_CHEAT;
			}
		}
	}
}

void CGameContext::ConCredits(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Teeworlds Team takes most of the credits also");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "This mod was originally created by \'3DA\'");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Now it is maintained & re-coded by:");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "\'[Egypt]GreYFoX@GTi\' and \'[BlackTee]den\'");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Others Helping on the code: \'heinrich5991\', \'noother\', \'LemonFace\', \'<3 fisted <3\' & \'Trust o_0 Aeeeh ?!\'");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Documentation: Zeta-Hoernchen, Entities: Fisico");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Code (in the past): \'3DA\' and \'Fluxid\'");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Modded mod my F.E.A.R.");
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Infos at www.di3el.ucoz.ru.");
}

void CGameContext::ConInfo(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "-->dDDRace<--"); 
        pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Mod by F.E.A.R."); 
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "dDDRace Mod. Version: " XXL_VERSION); 
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Download and infos at www.di3el.ucoz.ru"); 
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "For commands /cmdlist"); 
}

void CGameContext::ConHelp(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	if(pResult->NumArguments() == 0)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "/cmdlist will show a list of all chat commands");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "/help + any command will show you the help for this command");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Example /help settings will display the help about ");
	}
	else
	{
		const char *pArg = pResult->GetString(0);
		IConsole::CCommandInfo *pCmdInfo = pSelf->Console()->GetCommandInfo(pArg, CFGFLAG_SERVER);
		if(pCmdInfo && pCmdInfo->m_pHelp)
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", pCmdInfo->m_pHelp);
		else
				pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Command is either unknown or you have given a blank command without any parameters.");
	}
}

void CGameContext::ConSettings(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	if(pResult->NumArguments() == 0)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "to check a server setting say /settings and setting's name, setting names are:");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "teams, cheats, collision, hooking, endlesshooking, me, ");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "hitting, oldlaser, timeout, votes, pause and scores");
	}
	else
	{
		const char *pArg = pResult->GetString(0);
		char aBuf[256];
		float ColTemp;
		float HookTemp;
		pSelf->m_Tuning.Get("player_collision", &ColTemp);
		pSelf->m_Tuning.Get("player_hooking", &HookTemp);
		if(str_comp(pArg, "teams") == 0)
		{
			str_format(aBuf, sizeof(aBuf), "%s %s", g_Config.m_SvTeam == 1 ? "Teams are available on this server" : !g_Config.m_SvTeam ? "Teams are not available on this server" : "You have to be in a team to play on this server", /*g_Config.m_SvTeamStrict ? "and if you die in a team all of you die" : */"and if you die in a team only you die");
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
		}
		else if(str_comp(pArg, "collision") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", ColTemp?"Players can collide on this server":"Players Can't collide on this server");
		}
		else if(str_comp(pArg, "hooking") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", HookTemp?"Players can hook each other on this server":"Players Can't hook each other on this server");
		}
		else if(str_comp(pArg, "endlesshooking") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvEndlessDrag?"Players can hook time is unlimited":"Players can hook time is limited");
		}
		else if(str_comp(pArg, "hitting") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvHit?"Player's weapons affect each other":"Player's weapons has no affect on each other");
		}
		else if(str_comp(pArg, "oldlaser") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvOldLaser?"Lasers can hit you if you shot them and that they pull you towards the bounce origin (Like DDRace Beta)":"Lasers can't hit you if you shot them, and they pull others towards the shooter");
		}
		else if(str_comp(pArg, "me") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvSlashMe?"Players can use /me commands the famous IRC Command":"Players Can't use the /me command");
		}
		else if(str_comp(pArg, "timeout") == 0)
		{
			str_format(aBuf, sizeof(aBuf), "The Server Timeout is currently set to %d", g_Config.m_ConnTimeout);
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
		}
		else if(str_comp(pArg, "votes") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvVoteKick?"Players can use Callvote menu tab to kick offenders":"Players Can't use the Callvote menu tab to kick offenders");
			if(g_Config.m_SvVoteKick)
				str_format(aBuf, sizeof(aBuf), "Players are banned for %d second(s) if they get voted off", g_Config.m_SvVoteKickBantime);
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvVoteKickBantime?aBuf:"Players are just kicked and not banned if they get voted off");
		}
		else if(str_comp(pArg, "pause") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvPauseable?g_Config.m_SvPauseTime?"/pause is available on this server and it pauses your time too":"/pause is available on this server but it doesn't pause your time":"/pause is NOT available on this server");
		}
		else if(str_comp(pArg, "scores") == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvHideScore?"Scores are private on this server":"Scores are public on this server");
		}
	}
}

void CGameContext::ConRules(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	bool Printed = false;
	if(g_Config.m_SvDDRaceRules)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No blocking.");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No insulting / spamming.");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No fun voting / vote spamming.");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Breaking any of these rules will result in a penalty, decided by server admins.");
		Printed = true;
	}
	if(g_Config.m_SvRulesLine1[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine1);
		Printed = true;
	}
	if(g_Config.m_SvRulesLine2[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine2);
		Printed = true;
	}
	if(g_Config.m_SvRulesLine3[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine3);
		Printed = true;
	}
	if(g_Config.m_SvRulesLine4[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine4);
		Printed = true;
	}
	if(g_Config.m_SvRulesLine5[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine5);
		Printed = true;
	}
	if(g_Config.m_SvRulesLine6[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine6);
		Printed = true;
	}
	if(g_Config.m_SvRulesLine7[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine7);
		Printed = true;
	}
	if(g_Config.m_SvRulesLine8[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine8);
		Printed=true;
	}
	if(g_Config.m_SvRulesLine9[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine9);
		Printed=true;
	}
	if(g_Config.m_SvRulesLine10[0])
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", g_Config.m_SvRulesLine10);
		Printed = true;
	}
	if(!Printed)
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No Rules Defined, Kill em all!!");
}

void CGameContext::ConKill(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];

	if(!pPlayer || (pPlayer->m_LastKill && pPlayer->m_LastKill + pSelf->Server()->TickSpeed() * g_Config.m_SvKillDelay > pSelf->Server()->Tick()))
		return;

	pPlayer->m_LastKill = pSelf->Server()->Tick();
	pPlayer->KillCharacter(WEAPON_SELF);
	//pPlayer->m_RespawnTick = pSelf->Server()->Tick() + pSelf->Server()->TickSpeed() * g_Config.m_SvSuicidePenalty;
}

void CGameContext::ConTogglePause(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;

	if(g_Config.m_SvPauseable)
	{
		CCharacter* pChr = pPlayer->GetCharacter();
		if(!pPlayer->GetTeam() && pChr && (!pChr->GetWeaponGot(WEAPON_NINJA) || pChr->m_FreezeTime) && pChr->IsGrounded() && pChr->m_Pos==pChr->m_PrevPos && !pChr->Team() && !pPlayer->m_InfoSaved)
		{
			if(pPlayer->m_Last_Pause + pSelf->Server()->TickSpeed() * g_Config.m_SvPauseFrequency <= pSelf->Server()->Tick()) 
			{
				pPlayer->SaveCharacter();
				pPlayer->SetTeam(TEAM_SPECTATORS);
				pPlayer->m_InfoSaved = true;
				pPlayer->m_Last_Pause = pSelf->Server()->Tick();
			}
			else
				pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can\'t pause that often.");
		}
		else if(pPlayer->GetTeam()==TEAM_SPECTATORS && pPlayer->m_InfoSaved)
		{
			pPlayer->m_InfoSaved = false;
			pPlayer->m_PauseInfo.m_Respawn = true;
			pPlayer->SetTeam(TEAM_RED);
			//pPlayer->LoadCharacter();//TODO:Check if this system Works
		}
		else if(pChr)
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", (pChr->Team())?"You can't pause while you are in a team":pChr->GetWeaponGot(WEAPON_NINJA)?"You can't use /pause while you are a ninja":(!pChr->IsGrounded())?"You can't use /pause while you are a in air":"You can't use /pause while you are moving");
		else
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "No pause data saved.");
	}
	else
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Pause isn't allowed on this server.");
}

void CGameContext::ConTop5(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;

	if(g_Config.m_SvHideScore)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Showing the top 5 is not allowed on this server.");
		return;
	}

		if(pResult->NumArguments() > 0)
			pSelf->Score()->ShowTop5(pResult, pPlayer->GetCID(), pResult->GetInteger(0));
		else
			pSelf->Score()->ShowTop5(pResult, pPlayer->GetCID());
}
#if defined(CONF_SQL)
void CGameContext::ConTimes(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	if(g_Config.m_SvUseSQL)
	{
		CGameContext *pSelf = (CGameContext *)pUserData;
		CSqlScore *pScore = (CSqlScore *)pSelf->Score();
		CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
		if(!pPlayer)
			return;

		if(pResult->NumArguments() == 0)
		{
			pScore->ShowTimes(pPlayer->GetCID(),1);
			return;
		}

		else if(pResult->NumArguments() < 3)
		{
			if (pResult->NumArguments() == 1)
			{
				if(pResult->GetInteger(0) != 0)
					pScore->ShowTimes(pPlayer->GetCID(),pResult->GetInteger(0));
				else
					pScore->ShowTimes(pPlayer->GetCID(), (str_comp(pResult->GetString(0), "me") == 0) ? pSelf->Server()->ClientName(ClientID) : pResult->GetString(0),1);
				return;
			}
			else if (pResult->GetInteger(1) != 0)
			{
				pScore->ShowTimes(pPlayer->GetCID(), (str_comp(pResult->GetString(0), "me") == 0) ? pSelf->Server()->ClientName(ClientID) : pResult->GetString(0),pResult->GetInteger(1));
				return;
			}
		}
			
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "/times needs 0, 1 or 2 parameter. 1. = name, 2. = start number");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Example: /times, /times me, /times Hans, /times \"Papa Smurf\" 5");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Bad: /times Papa Smurf 5 # Good: /times \"Papa Smurf\" 5 ");						
	}	
}
#endif

void CGameContext::ConRank(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;

	if(pResult->NumArguments() > 0)
		if(!g_Config.m_SvHideScore)
			pSelf->Score()->ShowRank(ClientID, pResult->GetString(0), true);
		else
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Showing the rank of other players is not allowed on this server.");
	else
		pSelf->Score()->ShowRank(ClientID, pSelf->Server()->ClientName(ClientID));
}

void CGameContext::ConJoinTeam(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(pSelf->m_VoteCloseTime && pSelf->m_VoteCreator == ClientID)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You are running a vote please try again after the vote is done!");
		return;
	}
	else if(g_Config.m_SvTeam == 0)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Admin has disabled teams");
		return;
	}
	else if (g_Config.m_SvTeam == 2)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You must join to any team and play with anybody or you will not play");
	}
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];

	if(pResult->NumArguments() > 0)
	{
		if(pPlayer->GetCharacter() == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can't change teams while you are dead/a spectator.");
		}
		else
		{
			if(((CGameControllerDDRace*)pSelf->m_pController)->m_Teams.SetCharacterTeam(pPlayer->GetCID(), pResult->GetInteger(0)))
			{
				if(pPlayer->m_Last_Team + pSelf->Server()->TickSpeed() * g_Config.m_SvTeamChangeDelay <= pSelf->Server()->Tick())
				{
					char aBuf[512];
					str_format(aBuf, sizeof(aBuf), "%s joined team %d", pSelf->Server()->ClientName(pPlayer->GetCID()), pResult->GetInteger(0));
					pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
					pPlayer->m_Last_Team = pSelf->Server()->Tick();
				}
				else
				{
					pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can\'t join teams that fast!");
				}
			}
			else
			{
				pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You cannot join this team at this time");
			}
		}
	}
	else
	{
		char aBuf[512];
		if(pPlayer->GetCharacter() == 0)
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "You can't check your team while you are dead/a spectator.");
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "You are in team %d", pPlayer->GetCharacter()->Team());
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
		}
	}
}

void CGameContext::ConMe(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[256 + 24];

	str_format(aBuf, 256 + 24, "'%s' %s", pSelf->Server()->ClientName(ClientID), pResult->GetString(0));
	if(g_Config.m_SvSlashMe)
		pSelf->SendChat(-2, CGameContext::CHAT_ALL, aBuf, ClientID);
	else
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "/me is disabled on this server, admin can enable it by using sv_slash_me");
}

void CGameContext::ConToggleEyeEmote(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	pChr->m_EyeEmote = !pChr->m_EyeEmote;
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", (pChr->m_EyeEmote) ? "You can now use the preset eye emotes." : "You don't have any eye emotes, remember to bind some. (until you die)");
}

void CGameContext::ConToggleBroadcast(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	pChr->m_BroadCast = !pChr->m_BroadCast;
}

void CGameContext::ConEyeEmote(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if (pResult->NumArguments() == 0)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Emote commands are: /emote surprise /emote blink /emote close /emote angry /emote happy /emote pain");
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Example: /emote surprise 10 for 10 seconds or /emote surprise (default 1 second)");
	}
	else
	{
		if (pChr)
		{
			if (!str_comp(pResult->GetString(0), "angry"))
				pChr->m_DefEmote = EMOTE_ANGRY;
			else if (!str_comp(pResult->GetString(0), "blink"))
				pChr->m_DefEmote = EMOTE_BLINK;
			else if (!str_comp(pResult->GetString(0), "close"))
				pChr->m_DefEmote = EMOTE_BLINK;
			else if (!str_comp(pResult->GetString(0), "happy"))
				pChr->m_DefEmote = EMOTE_HAPPY;
			else if (!str_comp(pResult->GetString(0), "pain"))
				pChr->m_DefEmote = EMOTE_PAIN;
			else if (!str_comp(pResult->GetString(0), "surprise"))
				pChr->m_DefEmote = EMOTE_SURPRISE;
			else if (!str_comp(pResult->GetString(0), "normal"))
				pChr->m_DefEmote = EMOTE_NORMAL;
			else
				pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Unknown emote... Say /emote");

			int Duration = 1;
			if (pResult->NumArguments() > 1)
				Duration = pResult->GetInteger(1);

			pChr->m_DefEmoteReset = pSelf->Server()->Tick() + Duration * pSelf->Server()->TickSpeed();
		}
	}
}

void CGameContext::ConShowOthers(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;
	if(g_Config.m_SvShowOthers)
	{
		if(pPlayer->m_IsUsingDDRaceClient)
		{
			if(pResult->NumArguments())
				pPlayer->m_ShowOthers = pResult->GetInteger(0);
			else
				pPlayer->m_ShowOthers = !pPlayer->m_ShowOthers;
		}
		else
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Showing players from other teams is only available with DDRace Client, http://DDRace.info");
	}
	else
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Showing players from other teams is disabled by the server admin");
}

void CGameContext::Mute(IConsole::IResult *pResult, NETADDR *Addr, int Secs, const char *pDisplayName)
{
	char aBuf[128];
	int Found = 0;
	// find a matching mute for this ip, update expiration time if found
	for(int i = 0; i < m_NumMutes; i++)
	{
		if(net_addr_comp(&m_aMutes[i].m_Addr, Addr) == 0)
		{
			m_aMutes[i].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			Found = 1;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumMutes < MAX_MUTES)
		{
			m_aMutes[m_NumMutes].m_Addr = *Addr;
			m_aMutes[m_NumMutes].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			m_NumMutes++;
			Found = 1;
		}
	}
	if(Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been muted for %d seconds.", pDisplayName, Secs);
		SendChat(-1, CHAT_ALL, aBuf);
	}
	else if(pResult)// no free slot found
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", "mute array is full");
}

void CGameContext::ConMute(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", "Use either 'muteid <client_id> <seconds>' or 'muteip <ip> <seconds>'");
}

// mute through client id
void CGameContext::ConMuteID(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	NETADDR Addr;
	pSelf->Server()->GetClientAddr(Victim, &Addr);

	pSelf->Mute(pResult, &Addr, clamp(pResult->GetInteger(0), 1, 86400), pSelf->Server()->ClientName(Victim));
}

// mute through ip, arguments reversed to workaround parsing
void CGameContext::ConMuteIP(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	NETADDR Addr;
	if(net_addr_from_str(&Addr, pResult->GetString(0)))
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", "Invalid network address to mute");
	}
	pSelf->Mute(pResult, &Addr, clamp(pResult->GetInteger(1), 1, 86400), pResult->GetString(0));
}

// unmute by mute list index
void CGameContext::ConUnmute(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aIpBuf[64];
	char aBuf[64];
	int Victim = pResult->GetVictim();

	if(Victim < 0 || Victim >= pSelf->m_NumMutes)
		return;

	pSelf->m_NumMutes--;
	pSelf->m_aMutes[Victim] = pSelf->m_aMutes[pSelf->m_NumMutes];

	net_addr_str(&pSelf->m_aMutes[Victim].m_Addr, aIpBuf, sizeof(aIpBuf));
	str_format(aBuf, sizeof(aBuf), "Unmuted %s" , aIpBuf);
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", aBuf);
}

// list mutes
void CGameContext::ConMutes(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aIpBuf[64];
	char aBuf[128];
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", "Active mutes:");
	for(int i = 0; i < pSelf->m_NumMutes; i++)
	{
		net_addr_str(&pSelf->m_aMutes[i].m_Addr, aIpBuf, sizeof(aIpBuf));
		str_format(aBuf, sizeof aBuf, "%d: \"%s\", %d seconds left", i, aIpBuf, (pSelf->m_aMutes[i].m_Expire - pSelf->Server()->Tick()) / pSelf->Server()->TickSpeed());
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", aBuf);
	}
}

//Restored
void CGameContext::ConHammer(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	char aBuf[128];
	int Type = pResult->GetInteger(0);

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);

	if(!pChr)
		return;

	CServer* pServ = (CServer*)pSelf->Server();
	if(Type>10 || Type<0)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "Select hammer between 0 and 10");
	}
	else
	{
		pChr->m_HammerType = Type;
		pChr->m_DDRaceState = DDRACE_CHEAT;
		str_format(aBuf, sizeof(aBuf), "Hammer of '%s' ClientID=%d setted to %d", pServ->ClientName(Victim), Victim, Type);
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConToggleFly(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if(!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;
	if(pChr->m_Super)
	{
		pChr->m_Fly = !pChr->m_Fly;
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", (pChr->m_Fly) ? "Fly enabled" : "Fly disabled");
	}
}

void CGameContext::ConFreeze(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Seconds = -1;
	int Victim = pResult->GetVictim();

	char aBuf[128];

	if(pResult->NumArguments())
		Seconds = clamp(pResult->GetInteger(0), -2, 9999);

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;

	if(pSelf->m_apPlayers[Victim])
	{
		pChr->Freeze(Seconds);
		pChr->GetPlayer()->m_RconFreeze = Seconds != -2;
		CServer* pServ = (CServer*)pSelf->Server();
		if(Seconds >= 0)
			str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d has been Frozen for %d.", pServ->ClientName(Victim), Victim, Seconds);
		else if(Seconds == -2)
		{
			pChr->m_DeepFreeze = true;
			str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d has been Deep Frozen.", pServ->ClientName(Victim), Victim);
		}
		else
			str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d is Frozen until you unfreeze him.", pServ->ClientName(Victim), Victim);
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}

}

void CGameContext::ConUnFreeze(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	static bool Warning = false;
	char aBuf[128];
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if(!pChr)
		return;
	if(pChr->m_DeepFreeze && !Warning)
	{
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "warning", "This client is deeply frozen, repeat the command to defrost him.");
		Warning = true;
		return;
	}
	if(pChr->m_DeepFreeze && Warning)
	{
		pChr->m_DeepFreeze = false;
		Warning = false;
	}
	pChr->m_FreezeTime = 2;
	pChr->GetPlayer()->m_RconFreeze = false;
	CServer* pServ = (CServer*)pSelf->Server();
	str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d has been defrosted.", pServ->ClientName(Victim), Victim);
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
}

void CGameContext::ConInvis(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[128];
	int Victim = pResult->GetVictim();

	if(!pSelf->m_apPlayers[ClientID])
		return;

	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Invisible = true;
		CServer* pServ = (CServer*)pSelf->Server();
		str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d is now invisible.", pServ->ClientName(Victim), Victim);
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

void CGameContext::ConVis(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	if(!pSelf->m_apPlayers[ClientID])
		return;
	char aBuf[128];
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Invisible = false;
		CServer* pServ = (CServer*)pSelf->Server();
		str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d is visible.", pServ->ClientName(Victim), Victim);
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
}

//XXLmod
void CGameContext::ConSkin(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	const char *Skin = pResult->GetString(0);
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();;

	if(!pChr)
		return;
	//change skin
	str_copy(pSelf->m_apPlayers[Victim]->m_TeeInfos.m_SkinName, Skin, sizeof(pSelf->m_apPlayers[Victim]->m_TeeInfos.m_SkinName));
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s's skin changed to %s" ,pSelf->Server()->ClientName(Victim), Skin);
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
}

void CGameContext::ConRename(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	const char *newName = pResult->GetString(0);
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr =pSelf->m_apPlayers[Victim]->GetCharacter();

	if(!pChr)
		return;

	//change name
	char oldName[MAX_NAME_LENGTH];
	str_copy(oldName, pSelf->Server()->ClientName(Victim), MAX_NAME_LENGTH);

	pSelf->Server()->SetClientName(Victim, newName);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s has changed %s's name to '%s'" ,pSelf->Server()->ClientName(ClientID), oldName, pSelf->Server()->ClientName(Victim));
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

	str_format(aBuf, sizeof(aBuf), "%s changed your name to %s.", pSelf->Server()->ClientName(ClientID), pSelf->Server()->ClientName(Victim));
	pSelf->SendChatTarget(Victim, aBuf);
}

void CGameContext::ConOrgname(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);

	if(!pChr)
		return;

	//faked name
	char oldName[MAX_NAME_LENGTH];
	str_copy(oldName, pSelf->Server()->ClientName(Victim), MAX_NAME_LENGTH);

	//change name back
	pSelf->Server()->SetClientName(Victim, pSelf->m_apPlayers[Victim]->m_OrginalName);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s has changed %s's name back to '%s'" ,pSelf->Server()->ClientName(ClientID), oldName, pSelf->Server()->ClientName(Victim));
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);


	if (!pSelf->m_apPlayers[Victim]->m_isOrginalName){
		str_copy(pSelf->m_apPlayers[Victim]->m_OrginalName, oldName, MAX_NAME_LENGTH);
		pSelf->m_apPlayers[Victim]->m_isOrginalName = true;
	}
//			pSelf->m_apPlayers[Victim]->m_OrginalName = oldName;
//			str_copy(pSelf->m_apPlayers[Victim]->m_OrginalName, oldName, MAX_NAME_LENGTH);
	str_format(aBuf, sizeof(aBuf), "%s changed your name back to %s.", pSelf->Server()->ClientName(ClientID), pSelf->Server()->ClientName(Victim));
	pSelf->SendChatTarget(Victim, aBuf);

}

void CGameContext::ConFakeMessage(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	const char *message = pResult->GetString(0);
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();

	if(!pChr)
		return;
		//fake message
		pSelf->SendChat(Victim, CHAT_ALL, message);

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s has send a faked message from %s" ,pSelf->Server()->ClientName(ClientID), pSelf->Server()->ClientName(Victim));
		pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
}

void CGameContext::ConFastReload(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();
	if(!pChr)
		return;

	char aBuf[128];
	if (!pChr->m_FastReload)
	{
		pChr->m_ReloadMultiplier = 10000;
		pChr->m_FastReload = true;

		str_format(aBuf, sizeof(aBuf), "You got XXL by %s.", pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(Victim, aBuf);
	}
	else
	{
		pChr->m_ReloadMultiplier = 1000;
		pChr->m_FastReload = false;
		str_format(aBuf, sizeof(aBuf), "%s removed your XXL.", pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(Victim, aBuf);
	}

	pChr->m_DDRaceState = DDRACE_CHEAT;
}

void CGameContext::ConRainbow(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();
	int Rainbowtype = clamp(pResult->GetInteger(0), 0, 2);

	if(!pChr)
		return;

	char aBuf[256];
	if ((pSelf->m_apPlayers[Victim]->m_rainbow == RAINBOW_NONE || pSelf->m_apPlayers[Victim]->m_rainbow == RAINBOW_BLACKWHITE) && Rainbowtype <= 1)
	{
		pSelf->m_apPlayers[Victim]->m_rainbow = RAINBOW_COLOR;

		str_format(aBuf, sizeof(aBuf), "You got rainbow by %s.", pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(Victim, aBuf);
	}
	else if ((pSelf->m_apPlayers[Victim]->m_rainbow == RAINBOW_NONE || pSelf->m_apPlayers[Victim]->m_rainbow == RAINBOW_COLOR) && Rainbowtype == 2)
	{
		pSelf->m_apPlayers[Victim]->m_rainbow = RAINBOW_BLACKWHITE;

		str_format(aBuf, sizeof(aBuf), "You got black and white rainbow by %s.", pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(Victim, aBuf);
	}
	else
	{
		pSelf->m_apPlayers[Victim]->m_rainbow = RAINBOW_NONE;
		str_format(aBuf, sizeof(aBuf), "%s removed your rainbow.", pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(Victim, aBuf);
	}
}

void CGameContext::ConWhisper(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->m_apPlayers[ClientID]->GetCharacter();
	const char *message = pResult->GetString(0);

	if(!pChr)
		return;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s: %s" ,pSelf->Server()->ClientName(ClientID),message);
	pSelf->SendChatTarget(Victim, aBuf);

	str_format(aBuf, sizeof(aBuf), "-->%s: %s", pSelf->Server()->ClientName(ClientID),message);
	pSelf->SendChatTarget(ClientID, aBuf);
}

void CGameContext::ConHelper(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	int helper = 0;
	char aBuf[128];
	int Seconds;
	int HelperTime = 60;

	CGameContext *pSelf = (CGameContext *)pUserData;
	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	CServer* pServ = (CServer*)pSelf->Server();

	//Mods don't need helpers, they want to auth someone :-)
	int Victim = pResult->GetVictim();
	if (pSelf->m_apPlayers[ClientID]->m_Authed >= 2)
	{
		CCharacter* pChr2 = pSelf->GetPlayerChar(Victim);
		if(pSelf->m_apPlayers[Victim] && pChr2 && ClientID != Victim)
		{
			pSelf->m_apPlayers[Victim]->m_Authed = 1;
			pServ->SetRconLevel(Victim, 1);
		}
		else
		{
			pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", "A moder/admin/sadmin does not need help, to auth someone as helper use helper v");
		}
		return;
	}
	if(pChr && g_Config.m_SvHelper)
	{

		if (pSelf->m_apPlayers[ClientID]->m_Helped)
		{

			Seconds = pSelf->m_apPlayers[ClientID]->m_Helped / pSelf->Server()->TickSpeed();
			str_format(aBuf, sizeof(aBuf), "Please wait %d seconds to call a helper again.",  Seconds);
			pSelf->SendChatTarget(ClientID, aBuf);
			return;
		}
		for(int i = 0;i <=(int)MAX_CLIENTS-1 ; i++)
		{
			CCharacter* pChr3 = pSelf->GetPlayerChar(i);
			if(pChr3)
			{
				if (pSelf->m_apPlayers[i]->m_Authed>=1)
				{
					str_format(aBuf, sizeof(aBuf), "%s needs help. ID:%i" ,pSelf->Server()->ClientName(ClientID),ClientID);
					//~ str_format(aBuf, sizeof(aBuf), "%s: %i ID:%i" ,pSelf->Server()->ClientName(i),pSelf->m_apPlayers[i]->m_Authed,i);
					//~ pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
					pSelf->SendChatTarget(i, aBuf);
					pServ->SendRconLine(i, aBuf);
					helper++;

				}
			}
		}
		if (helper == 0 )
			str_format(aBuf, sizeof(aBuf), "Sorry, but there is no helper online.");
		else if (helper == 1)
		{
			str_format(aBuf, sizeof(aBuf), "You called one helper!");
			pSelf->m_apPlayers[ClientID]->m_Helped = HelperTime * pSelf->Server()->TickSpeed();
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "%i helpers called!", helper);
			pSelf->m_apPlayers[ClientID]->m_Helped = HelperTime * pSelf->Server()->TickSpeed();
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "sv_helper not activated.");
	}
	pSelf->SendChatTarget(ClientID, aBuf);
}

void CGameContext::ConScore(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	int Victim = pResult->GetVictim();
	int Score = clamp(pResult->GetInteger(0), -9999, 9999);

	CGameContext *pSelf = (CGameContext *)pUserData;
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();;
	if(!pChr)
		return;

	//~ pSelf->m_apPlayers[ClientID]->m_Score = (Score()->PlayerData(ClientID)->m_BestTime)?Score()->PlayerData(ClientID)->m_BestTime:0;
	pSelf->m_apPlayers[Victim]->m_Score = Score;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s set score of %s to %i" ,pSelf->Server()->ClientName(ClientID),pSelf->Server()->ClientName(Victim), Score);
	pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

	//~ char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s set your to %i.", pSelf->Server()->ClientName(ClientID), Score);
	pSelf->SendChatTarget(Victim, aBuf);
}

void CGameContext::ConBlood(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	int Victim = pResult->GetVictim();

	CGameContext *pSelf = (CGameContext *)pUserData;
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();;
	if(!pChr)
		return;

	char aBuf[128];
	if (!pChr->m_Bloody)
	{
		pChr->m_Bloody = true;

		str_format(aBuf, sizeof(aBuf), "You got bloody by %s.", pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(Victim, aBuf);
	}
	else
	{
		pChr->m_Bloody = false;
		str_format(aBuf, sizeof(aBuf), "%s removed your blood.", pSelf->Server()->ClientName(ClientID));
		pSelf->SendChatTarget(Victim, aBuf);
	}
}

void CGameContext::ConRescue(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CCharacter* pChr = pSelf->m_apPlayers[ClientID]->GetCharacter();
	char aBuf[256];

	if (!g_Config.m_SvRescue){
		pSelf->SendChatTarget(ClientID, "Rescue is not activated.");
		return;
	}

	if(pChr)
	{
		if (!pChr->m_LastRescue){

			float RescueDelay = 0.15;
			if (pChr->m_FreezeTime == 0)
			{
				str_format(aBuf, sizeof(aBuf), "You are not freezed!");
				pSelf->SendChatTarget(ClientID, aBuf);
			}
			else if (pChr->m_DeepFreeze)
			{
				str_format(aBuf, sizeof(aBuf), "You are deepfreezed, undeepfreeze first!");
				pSelf->SendChatTarget(ClientID, aBuf);
			}
			else if (!pChr->IsAlive())
			{
				str_format(aBuf, sizeof(aBuf), "You are not alive!");
				pSelf->SendChatTarget(ClientID, aBuf);
			}
			else
			{
				//not freezed
				for(int i = 0;i <=(int)MAX_CLIENTS-1 ; i++)
				{
					if ( pSelf->m_apPlayers[i])
					{
						CCharacter* pChr2 = pSelf->m_apPlayers[i]->GetCharacter();
						//who hooks me?
						if (pChr2 && pChr2->Core()->m_HookedPlayer == ClientID)
						{
							//Release hook
							pChr2->Core()->m_HookedPlayer = -1;
							pChr2->Core()->m_HookState = HOOK_RETRACTED;
							pChr2->Core()->m_HookPos = pChr2->Core()->m_Pos;
						}
					}
				}
				if(g_Config.m_SvRescueEffects)
				{
					//Blood effect
					pChr->GameServer()->CreateDeath(pChr->m_Pos, ClientID);
					//Spawn effect
					pChr->GameServer()->CreatePlayerSpawn(pChr->m_RescuePos);
				}
				//"save" last rescue time
				pChr->m_LastRescue = RescueDelay * pSelf->Server()->TickSpeed();
				//Teleport player
				pChr->Core()->m_Pos = pChr->m_RescuePos;
			}
			//str_format(aBuf, sizeof(aBuf), "HookedPlayer: %d",pChr->Core()->m_HookedPlayer );
			//pSelf->SendChatTarget(ClientID, aBuf);
		}
		else
		{
			pChr->UnFreeze();
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "You are not alive!");
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConRegister(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MemberList->Register(pResult, ClientID, pResult->GetString(0), pSelf);
}

void CGameContext::ConLogin(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MemberList->Login(pResult, ClientID, pResult->GetString(0), pSelf);
}

void CGameContext::ConCheckMember(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MemberList->Check(pResult->GetVictim(), pSelf);
}

void CGameContext::ConMember(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MemberList->Member(pResult->GetVictim(), pSelf);
}

void CGameContext::ConUnMember(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->MemberList->UnMember(pResult->GetVictim(), pSelf);
}

void CGameContext::ConIceHammer(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();

	if (!pChr)
		return;

	pChr->m_IceHammer = true;
}

void CGameContext::ConUnIceHammer(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter* pChr = pSelf->m_apPlayers[Victim]->GetCharacter();

	if (!pChr)
		return;

	pChr->m_IceHammer = false;
}

void CGameContext::ConSetlvl4(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CServer* pServ = (CServer*)pSelf->Server();
	if(pSelf->m_apPlayers[Victim])
	{
		pSelf->m_apPlayers[Victim]->m_Authed = 4;
		pServ->SetRconLevel(Victim, 4);
	}
}


void CGameContext::ConTest(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	int Victim = pResult->GetVictim();
	const char *message = pResult->GetString(0);
	char aBuf[256];

	CGameContext *pSelf = (CGameContext *)pUserData;
	CCharacter* pChr = pSelf->m_apPlayers[ClientID]->GetCharacter();
	if(!pChr)
		return;

//	str_format(aBuf, sizeof(aBuf), "md5:%s" , md5(message));
//
//	pSelf->SendChatTarget(Victim, aBuf);

	//pChr->m_is_bloody = 1;
	//~ pSelf->m_apPlayers[ClientID]->m_Score = (Score()->PlayerData(ClientID)->m_BestTime)?Score()->PlayerData(ClientID)->m_BestTime:0;
	//~ pSelf->m_apPlayers[ClientID]->m_Score = 123;
	//~ char aBuf[256];
	//~ str_format(aBuf, sizeof(aBuf), "%s: %s" ,pSelf->Server()->ClientName(ClientID),message);
	//~ str_format(aBuf, sizeof(aBuf), "Teeinfo:%i" , pSelf->m_apPlayers[ClientID]->m_TeeInfos.m_ColorBody);
	//~ pResult->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	//~ pSelf->SendChatTarget(Victim, aBuf);

}

void CGameContext::ConSetJumps(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();
	CCharacter *pChr = pSelf->m_apPlayers[Victim]->GetCharacter();
	if (!pChr)
		return;

	pChr->Core()->m_max_jumps = clamp(pResult->GetInteger(0), 0, 9999);
}

void CGameContext::ConJumps(IConsole::IResult *pResult, void *pUserData, int ClientID)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CCharacter *pChr = pSelf->m_apPlayers[ClientID]->GetCharacter();
	if (!pChr)
		return;

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "You can jump %d times.", pChr->Core()->m_max_jumps);
	pSelf->SendChatTarget(ClientID, aBuf);
}


