#include "hooks.h"

#include "cs2ac.h"
#include "movement_analysis/player_context.h"
#include "movement_analysis/events/movement_events.h"
#include "utils/ctimer.h"
#include "utils/gameconfig.h"
#include "utils/interfaces.h"
#include "utils/utils.h"

#include "igameevents.h"
#include "iserver.h"
#include "cs_gameevents.pb.h"

// Metamod 2.0 build 1461+ (0cc4e20) dropped SourceHook for plugins; hooks use KHook, as in cs2kz c53844b.
namespace
{
	// KHook keeps a vtable hook installed per vtable; Add/Remove only choose which pawns reach the callback.
	CBaseEntity *teleportPawns[MAXPLAYERS] {};

	struct PendingGameEvent
	{
		IGameEvent *event;
		MovementPlayer *player;
	};

	std::vector<PendingGameEvent> pendingGameEvents;
	bool AddTeleportHook(MovementPlayer *player);

	bool IsConsumedEvent(IGameEvent *event)
	{
		if (!event)
		{
			return false;
		}
		const char *name = event->GetName();
		return CS2AC_STREQ(name, "weapon_fire") || CS2AC_STREQ(name, "player_hurt") || CS2AC_STREQ(name, "player_death")
			   || CS2AC_STREQ(name, "player_spawn") || CS2AC_STREQ(name, "smokegrenade_detonate");
	}

	MovementPlayer *ResolveEventPlayer(IGameEvent *event)
	{
		if (!event)
		{
			return nullptr;
		}
		auto *player = g_pCS2ACPlayerManager->ToPlayer(static_cast<CBasePlayerController *>(event->GetPlayerController("userid")));
		if (player)
		{
			return player;
		}

		int userID = event->GetInt("userid", -1);
		return userID < 0 ? nullptr : g_pCS2ACPlayerManager->ToPlayer(CPlayerUserId(userID));
	}

	KHook::Return<bool> HookFireEventBefore(IGameEventManager2 *, IGameEvent *event, bool)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore, true};
		}
		PendingGameEvent pending {};
		if (IsConsumedEvent(event))
		{
			pending = {interfaces::pGameEventManager->DuplicateEvent(event), ResolveEventPlayer(event)};
		}
		pendingGameEvents.push_back(pending);
		return {KHook::Action::Ignore, true};
	}

	KHook::Return<void> HookPostEvent(IGameEventSystem *, CSplitScreenSlot, bool, int, const uint64 *, INetworkMessageInternal *event,
									  const CNetMessage *data, unsigned long, NetChannelBufType_t)
	{
		if (!g_CS2AC.IsLoaded() || !event || !data)
		{
			return {KHook::Action::Ignore};
		}
		auto *info = event->GetNetMessageInfo();
		if (!info)
		{
			return {KHook::Action::Ignore};
		}
		if (info->m_MessageId == GE_FireBulletsId)
		{
			g_CS2AC.OnFireBullets(*data->ToPB<CMsgTEFireBullets>());
		}
		return {KHook::Action::Ignore};
	}

	KHook::Return<bool> HookFireEventAfter(IGameEventManager2 *, IGameEvent *, bool)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore, true};
		}
		PendingGameEvent pending {};
		if (!pendingGameEvents.empty())
		{
			pending = pendingGameEvents.back();
			pendingGameEvents.pop_back();
		}
		if (pending.event)
		{
			g_CS2AC.OnGameEvent(pending.event, pending.player);
			if (CS2AC_STREQ(pending.event->GetName(), "player_spawn") && pending.player)
			{
				// A spawn can replace the pawn object after ClientActive, so bind the per-pawn hook again.
				pending.player->OnTeleport(nullptr, nullptr, nullptr);
				AddTeleportHook(pending.player);
			}
			interfaces::pGameEventManager->FreeEvent(pending.event);
		}
		return {KHook::Action::Ignore, true};
	}

	KHook::Return<void> HookTeleport(CBaseEntity *pawn, const Vector *origin, const QAngle *angles, const Vector *velocity)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore};
		}
		auto *current = g_pCS2ACPlayerManager->ToPlayer(static_cast<CBasePlayerPawn *>(pawn));
		if (current)
		{
			current->OnTeleport(origin, angles, velocity);
		}
		return {KHook::Action::Ignore};
	}

	KHook::Virtual<CBaseEntity, void, const Vector *, const QAngle *, const Vector *> teleportHook(HookTeleport, nullptr);

	bool RemoveTeleportHook(CPlayerSlot slot)
	{
		if (slot.Get() < 0 || slot.Get() >= MAXPLAYERS || !teleportPawns[slot.Get()])
		{
			return true;
		}
		teleportHook.Remove(teleportPawns[slot.Get()]);
		teleportPawns[slot.Get()] = nullptr;
		return true;
	}

	bool AddTeleportHook(MovementPlayer *player)
	{
		if (!player || !player->GetPlayerPawn())
		{
			return false;
		}
		RemoveTeleportHook(player->GetPlayerSlot());
		CBaseEntity *pawn = player->GetPlayerPawn();
		teleportHook.Add(pawn);
		teleportPawns[player->GetPlayerSlot().Get()] = pawn;
		return true;
	}

	KHook::Return<void> HookGameFrameBefore(ISource2Server *, bool, bool, bool)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore};
		}
		if (auto *globals = g_pCS2ACUtils->GetGlobals())
		{
			g_CS2AC.serverGlobals = *globals;
		}
		return {KHook::Action::Ignore};
	}

	KHook::Return<void> HookGameFrameAfter(ISource2Server *, bool simulating, bool, bool)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore};
		}
		if (auto *globals = g_pCS2ACUtils->GetGlobals())
		{
			g_CS2AC.serverGlobals = *globals;
		}
		g_CS2AC.OnGameFrame(simulating);
		ProcessTimers();
		MovementEventService::ActiveCheck();
		return {KHook::Action::Ignore};
	}

	KHook::Return<void> HookClientFullyConnect(ISource2GameClients *, CPlayerSlot slot)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore};
		}
		g_pCS2ACPlayerManager->OnClientFullyConnect(slot);
		g_ClientCvarValue.OnClientFullyConnected(slot, g_pCS2ACPlayerManager->ToPlayer(slot)->IsFakeClient());
		g_CS2AC.OnClientFullyConnect(slot);
		return {KHook::Action::Ignore};
	}

	KHook::Return<void> HookClientSettingsChanged(ISource2GameClients *, CPlayerSlot slot)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore};
		}
		g_CS2AC.OnClientSettingsChanged(slot);
		return {KHook::Action::Ignore};
	}

	KHook::Return<void> HookClientActive(ISource2GameClients *, CPlayerSlot slot, bool, const char *, uint64 xuid)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore};
		}
		g_pCS2ACPlayerManager->OnClientActive(slot, xuid);
		auto *player = g_pCS2ACPlayerManager->ToPlayer(slot);
		if (player && player->GetPlayerPawn())
		{
			AddTeleportHook(player);
		}
		return {KHook::Action::Ignore};
	}

	KHook::Return<void> HookClientDisconnect(ISource2GameClients *, CPlayerSlot slot, ENetworkDisconnectionReason, const char *, uint64,
											 const char *)
	{
		if (!g_CS2AC.IsLoaded())
		{
			return {KHook::Action::Ignore};
		}
		RemoveTeleportHook(slot);
		g_ClientCvarValue.OnClientDisconnect(slot);
		g_CS2AC.OnClientDisconnect(slot);
		g_pCS2ACPlayerManager->OnClientDisconnect(slot);
		return {KHook::Action::Ignore};
	}

	KHook::Virtual<ISource2Server, void, bool, bool, bool> gameFrameHook(&ISource2Server::GameFrame, HookGameFrameBefore, HookGameFrameAfter);
	KHook::Virtual<ISource2GameClients, void, CPlayerSlot> clientFullyConnectHook(&ISource2GameClients::ClientFullyConnect, nullptr,
																				   HookClientFullyConnect);
	KHook::Virtual<ISource2GameClients, void, CPlayerSlot> clientSettingsChangedHook(&ISource2GameClients::ClientSettingsChanged, nullptr,
																					  HookClientSettingsChanged);
	KHook::Virtual<ISource2GameClients, void, CPlayerSlot, bool, const char *, uint64> clientActiveHook(&ISource2GameClients::ClientActive, nullptr,
																										 HookClientActive);
	KHook::Virtual<ISource2GameClients, void, CPlayerSlot, ENetworkDisconnectionReason, const char *, uint64, const char *>
		clientDisconnectHook(&ISource2GameClients::ClientDisconnect, nullptr, HookClientDisconnect);
	KHook::Virtual<IGameEventManager2, bool, IGameEvent *, bool> fireEventHook(&IGameEventManager2::FireEvent, HookFireEventBefore,
																			   HookFireEventAfter);
	KHook::Virtual<IGameEventSystem, void, CSplitScreenSlot, bool, int, const uint64 *, INetworkMessageInternal *, const CNetMessage *,
				   unsigned long, NetChannelBufType_t>
		postEventHook(&IGameEventSystem::PostEventAbstract, HookPostEvent, nullptr);

	template<typename HOOK, typename TARGET>
	void AttachHook(HOOK &hook, TARGET *target, const char *name, std::vector<std::string> &missing)
	{
		if (!target)
		{
			missing.emplace_back(std::string("The ") + name + " server hook could not be started.");
			return;
		}
		hook.Add(target);
	}

} // namespace

bool hooks::Initialize(std::vector<std::string> &missing)
{
	teleportHook.Configure(g_pGameConfig->GetOffset("Teleport"));

	AttachHook(gameFrameHook, interfaces::pServer, "game frame", missing);
	AttachHook(clientFullyConnectHook, g_pSource2GameClients, "fully connected player", missing);
	AttachHook(clientSettingsChangedHook, g_pSource2GameClients, "player setting update", missing);
	AttachHook(clientActiveHook, g_pSource2GameClients, "active player", missing);
	AttachHook(clientDisconnectHook, g_pSource2GameClients, "disconnecting player", missing);
	AttachHook(fireEventHook, interfaces::pGameEventManager, "game event", missing);
	AttachHook(postEventHook, interfaces::pGameEventSystem, "weapon telemetry", missing);

	if (!missing.empty())
	{
		Cleanup();
		return false;
	}
	return true;
}

void hooks::HookActivePlayers()
{
	for (i32 i = 1; i <= MAXPLAYERS; ++i)
	{
		auto *player = g_pCS2ACPlayerManager->ToPlayer(static_cast<u32>(i));
		if (player && player->IsInGame() && player->GetPlayerPawn())
		{
			AddTeleportHook(player);
		}
	}
}

bool hooks::ResetMap()
{
	for (i32 slot = 0; slot < MAXPLAYERS; ++slot)
	{
		RemoveTeleportHook(CPlayerSlot(slot));
	}
	return true;
}

bool hooks::Cleanup()
{
	// ClearHooks only detaches our targets; the vtable patches go away when Metamod unloads the plugin.
	ResetMap();
	teleportHook.ClearHooks();
	gameFrameHook.ClearHooks();
	clientFullyConnectHook.ClearHooks();
	clientSettingsChangedHook.ClearHooks();
	clientActiveHook.ClearHooks();
	clientDisconnectHook.ClearHooks();
	fireEventHook.ClearHooks();
	postEventHook.ClearHooks();
	if (interfaces::pGameEventManager)
	{
		for (const auto &pending : pendingGameEvents)
		{
			if (pending.event)
			{
				interfaces::pGameEventManager->FreeEvent(pending.event);
			}
		}
	}
	pendingGameEvents.clear();
	return true;
}
