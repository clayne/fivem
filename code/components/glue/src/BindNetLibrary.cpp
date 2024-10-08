/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <NetLibrary.h>
#include <CefOverlay.h>
#include <ICoreGameInit.h>
#include <GameInit.h>
#include <nutsnbolts.h>

#include <DrawCommands.h>
#include <FontRenderer.h>

#include <msgpack.hpp>

#include <CoreConsole.h>
#include <se/Security.h>

#include "ConVarsPacketHandler.h"

static InitFunction initFunction([] ()
{
	seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "system.internal" }, se::Object{ "builtin" }, se::AccessType::Allow);

	static NetLibrary* netLibrary;

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* library)
	{
		netLibrary = library;
		static std::string netLibWarningMessage;
		static std::mutex netLibWarningMessageLock;

		library->OnConnectOKReceived.Connect([](NetAddress)
		{
			std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
			netLibWarningMessage = "";
		});

		library->OnReconnectProgress.Connect([](const std::string& msg)
		{
			std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
			netLibWarningMessage = msg;
		});

		OnPostFrontendRender.Connect([]()
		{
			if (!netLibWarningMessage.empty())
			{
				std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
				TheFonts->DrawText(ToWide(netLibWarningMessage), CRect(40.0f, 40.0f, 800.0f, 500.0f), CRGBA(255, 0, 0, 255), 40.0f, 1.0f, "Comic Sans MS");
			}
		});

		library->OnStateChanged.Connect([] (NetLibrary::ConnectionState curState, NetLibrary::ConnectionState lastState)
		{
			if (curState == NetLibrary::CS_ACTIVE)
			{
				ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

				if (!gameInit->GetGameLoaded())
				{
					trace("^2Network connected, triggering initial game load...\n");

					gameInit->LoadGameFirstLaunch([]()
					{
						// download frame code
						Sleep(1);

						return netLibrary->AreDownloadsComplete();
					});
				}
				else
				{
					trace("^2Network connected, triggering game reload...\n");

					gameInit->ReloadGame();
				}
			}
		});

		OnKillNetwork.Connect([=] (const char* message)
		{
			{
				std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
				netLibWarningMessage = "";
			}

			library->Disconnect(message);

			Instance<ICoreGameInit>::Get()->ClearVariable("storyMode");
			Instance<ICoreGameInit>::Get()->ClearVariable("localMode");
		});

		OnKillNetworkDone.Connect([=]()
		{
			library->Disconnect();
		});

		// Process ConVar values sent from the server
		library->AddPacketHandler<fx::ConVarsPacketHandler>(true);
	});

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([]()
	{
		nui::SetMainUI(false);
		nui::DestroyFrame("mpMenu");

		auto context = (netLibrary->GetConnectionState() != NetLibrary::CS_IDLE) ? ("server_" + netLibrary->GetTargetContext()) : "game";
		nui::SwitchContext(context);
	});

	// #TODOLIBERTY: ?
#ifndef GTA_NY
	OnFirstLoadCompleted.Connect([] ()
	{
		g_gameInit.SetGameLoaded();
	});
#endif
});
