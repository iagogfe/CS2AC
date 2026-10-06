> [!IMPORTANT]
> **Hey, thanks for stopping by. This version is no longer maintained.**
>
> I owe you an explanation, because this repo went quiet without a word, and that wasn't fair to the people who used it.
>
> **What happened:** CS2AC and CS2FOW started here as open-source projects, and I loved building them in the open. First, I kept losing access to this GitHub account for weeks at a time, which made it impossible to keep the projects running here reliably. On top of that, people copied the code and sold it as their own without respecting the license. Cheat developers read the detection code to see exactly what gets caught. And keeping an anti-cheat alive for every CS2 update turned into a full-time job. So I moved development to closed source, where I could protect the work and keep going. I never stopped working on it.
>
> **If you still run this version:** please know it isn't updated for current CS2 builds anymore. It can break after a game update, or quietly protect less than you'd expect.
>
> **The current CS2AC and CS2FOW** are alive and getting better every week. You can try them free for 7 days at [karola3vax.com](https://karola3vax.com). And if you have questions, want help switching, or just want to say hi, come find me on [Discord](https://discord.com/invite/gaAUsKAuqQ).
>
> Thank you to everyone who starred, tested, reported bugs and believed in this project early on. It's a big part of why it still exists.

<div align="center">

<img src="docs/cs2ac-logo.png" width="760" alt="CS2AC">

<a href="https://buymeacoffee.com/karola3vax"><img src="docs/buymeacoffee-banner.png" width="760" alt="Support CS2AC development"></a>

### A server-side anti-cheat for Counter-Strike 2 community servers.

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-5c7cfa?style=for-the-badge)](#install)
[![License](https://img.shields.io/badge/license-AGPL--3.0-2ea44f?style=for-the-badge)](LICENSE)
[![Buy Me a Coffee](https://img.shields.io/badge/Buy_Me_a_Coffee-Support_Development-FFDD00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=000000)](https://buymeacoffee.com/karola3vax)

**A good match should be decided by the players, not by who brought the better cheat.**

</div>

## What it does

CS2AC runs on your server and watches what players send to it: where they aim, how they shoot, how they move, and their game settings. When it sees clear signs of cheating, it can announce it, report it to your Discord, and kick or ban the player through your admin plugin.

Players don't install anything. It only works on community servers you run yourself, not in Premier or Valve matchmaking.

## What it catches

**Aim**
- **Aimbot:** the crosshair snaps onto an enemy right before a hit.
- **Aimlock:** the crosshair stays glued to a moving enemy, even behind walls.
- **Triggerbot:** the gun fires the instant the crosshair touches an enemy.
- **Silent aim:** shots hit somewhere the player never actually looked.
- **Inhuman accuracy:** far more hits than any real player could land.
- **Irregular behavior:** rage-style jump shots and no-scopes that keep connecting.

**Movement**
- **Bhop:** perfectly timed jumps, landing after landing.
- **Autostrafe:** automatic left-right turning in the air to gain speed.
- **Hyperscroll:** an unnatural flood of jump presses.
- **Nulls:** a script switching A and D with perfect timing.

**Cheat traces**
- **Anti-aim:** impossible view angles, spinning or jittering.
- **DLL injection:** the game asking for things only cheats need.
- **Desubticking, subtick spam, invalid input:** movement data that a normal game never sends.
- **Doubletap:** the same gun firing twice at once.
- **Invalid CVar:** game settings changed to values that shouldn't be possible.
- **Namechanger:** rapid name changes to spam or confuse.

No anti-cheat catches everything. CS2AC can only judge what reaches the server; it never looks at a player's PC.

## See it in action

<table>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/aimbot.gif" width="100%" alt="CS2AC catching an aimbot snap">
<br><strong>Aimbot</strong>
</td>
<td width="50%" align="center">
<img src="docs/showcase/aimlock.gif" width="100%" alt="CS2AC catching an aimlock">
<br><strong>Aimlock</strong>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/antiaim.gif" width="100%" alt="CS2AC catching anti-aim">
<br><strong>Anti-aim</strong>
</td>
<td width="50%" align="center">
<img src="docs/showcase/bhop.gif" width="100%" alt="CS2AC catching a bhop script">
<br><strong>Bhop</strong>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/irregular-behavior.gif" width="100%" alt="CS2AC catching rage-style shots">
<br><strong>Irregular behavior</strong>
</td>
<td width="50%" align="center">
<img src="docs/showcase/silentaim.gif" width="100%" alt="CS2AC catching silent aim">
<br><strong>Silent aim</strong>
</td>
</tr>
</table>

When it catches someone, everyone sees a short message like this:

```text
[CS2AC] detected AIMBOT on Player and punished.
```

<div align="center">
<img src="docs/showcase/webhook.png" width="432" alt="A CS2AC detection report in Discord">
</div>

## Install

You need a Windows or Linux CS2 dedicated server with [Metamod:Source](https://www.sourcemm.net/) 2.x installed.

1. Download the Windows or Linux package from the **Releases** tab.
2. Unzip it into your server's main folder. It starts with a `game` folder, so the files land in the right places on their own.
3. Open `game/csgo/cfg/cs2ac.cfg` and adjust it if you like (see below).
4. Start the server.
5. Type `cs2ac_status` in the server console to check it's running.

This legacy version doesn't receive updates anymore, so it may stop working after a CS2 update.

## Settings you may want to change

Everything lives in `game/csgo/cfg/cs2ac.cfg`, and every option is explained there in plain words. After a change, type `cs2ac_reload`.

| Setting | What it does |
| --- | --- |
| `cs2ac_enabled` | Turns CS2AC on (`1`) or off (`0`). |
| `cs2ac_whitelist` | Steam IDs that can be detected but never punished, for example your admins. |
| `cs2ac_punishment_command` | The ban command it sends. Made for [CS2-SimpleAdmin](https://github.com/daffyyyy/CS2-SimpleAdmin) by default; change it if you use another admin plugin. |
| `cs2ac_kick_command` | The kick command, used for lighter cases. |
| `cs2ac_chat_announcements` / `cs2ac_center_announcements` | Show catches in chat and in the middle of the screen. |
| `cs2ac_language` | Language of the messages, for example `en`, `de`, `tr`, `ru`, `pt-br`. |
| `cs2ac_webhook_url` | A Discord webhook link, to get a report in your Discord for every catch. |

**Discord reports:** create a webhook in your Discord channel, paste its link into `cs2ac_webhook_url`, type `cs2ac_reload`, then `cs2ac_webhook_test` to try it.

## Useful console commands

| Command | What it does |
| --- | --- |
| `cs2ac_status` | Shows whether everything is working. |
| `cs2ac_reload` | Reloads the settings file. |
| `cs2ac_check_config` | Points out mistakes in your settings. |
| `cs2ac_test_announcement` | Shows a test message without catching anyone. |
| `cs2ac_webhook_test` | Sends a test report to your Discord. |

## Good to know

- **Players joining** see a short private message that the server is protected by CS2AC.
- **Whitelisted players** can still be detected and reported, but they're never kicked or banned.
- **Free-for-all modes** work too.

## Support

If CS2AC helped your server, you can support my work at [buymeacoffee.com/karola3vax](https://buymeacoffee.com/karola3vax).

## License

This legacy version is open source under the [AGPL-3.0 license](LICENSE). Libraries it uses keep their own licenses; see [third-party notices](THIRD_PARTY_NOTICES.md).
