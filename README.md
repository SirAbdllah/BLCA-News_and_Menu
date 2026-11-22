# BLCA News_and_Menu

Developed by **BLCA Development Team**.

BLCA News_and_Menu combines Arabic-capable news broadcasts, a styled pause menu, and server administration in one Arma Reforger addon. Broadcasts use a moving ticker, a shared BLC logo, sender labels, faction portraits, and fade animations. The ESC menu uses a translucent navy panel and blue controls, with administration entries shown according to the player's permissions.

The project is named **BLCA News_and_Menu**. The supplied logo retains the **BLC** lettering in the original artwork.

This folder contains the game addon project. The standalone Windows administration application, **BLCA Admin Manger**, is supplied in a separate folder of that name alongside the addon folder. Use a compatible manager release on the server computer with access to the server's actual profile folder.

## Moving existing server data

When upgrading an existing server, first stop the server, manager, and any background agent. Keep a copy of the current administration data folder in the actual server profile, then rename that folder to `BLCAAdminSuite` in the same location. Preserve its complete contents, including `config.json`, `config.rev`, `audit`, `manager-data`, bans, and other files. Run the updated addon together with BLCA Admin Manger 1.1.4, then select the profile or `BLCAAdminSuite` folder in the application. Existing permissions, bans, and history continue loading after this manual folder move.

To retain local folder selections, close the application and rename its current preferences folder under `%LOCALAPPDATA%` to `BLCAAdminManager`, preserving `settings.json`. You can also select the server profile and log folder again in the interface after updating.

## Installation and updates

1. Open the `addon.gproj` file directly inside this folder in Arma Reforger Tools / Workbench.
2. The project depends on the base game data, identified by `58D0FB3206B6F859`. Add the published addon to the server configuration and distribute the matching update to clients.
3. Start a scenario and locate the `BLCAAdminSuite` folder created in the server profile. The integration uses scenarios based on `SCR_BaseGameMode`.
4. In the separate Windows application, select the profile folder or `BLCAAdminSuite` itself. For a launch argument such as `-profile D:\Reforger\ServerA`, the actual folder may be `D:\Reforger\ServerA\profile\BLCAAdminSuite`. Select the folder the server actually creates.
5. Add staff in the manager, then open the in-game pause menu and select **Admin Panel** to activate an administration session.

The project uses GUID `C716458FF9B84A03`, technical project ID `BLCA_NEWS_MENU`, and data path `$profile:BLCAAdminSuite/`. Publish the update to the existing Workshop project from your account, then update the server and its clients.

Before replacing server data or importing an older configuration, copy the complete `BLCAAdminSuite` data folder. It contains `config.json`, `config.rev`, `audit`, and `manager-data`. A copy of the addon project alone does not contain the live staff configuration, bans, and player history.

## Connecting the manager and importing staff

Run `BLCA Admin Manger.exe` from the sibling `BLCA Admin Manger` folder, select the server profile, and choose **Connect**. The connection and configuration status appear at the top of the window. The application and addon exchange local files and need access to the same folder. This connection requires no network port or RCON configuration.

Add staff manually with the player's correct UUID from the Players page, or import an existing `ServerAdminTools_Config.json` from Settings. Import reads the `admins`, `gameMasters`, and `bans` collections and skips zero identities. Imported administrators receive GM, news, player listing, kick, ban, and GM-limit bypass permissions. Imported Game Masters receive GM and player listing. Review the individual permissions after importing.

The manager saves `config.json` and updates `config.rev`. The addon reads the new revision during its next update cycle. Configuration errors appear in the server status. If another manager window saves a newer revision during editing, the application rejects the stale save so the changes can be refreshed.

## Administration sessions and permissions

Listing a UUID as staff makes that player eligible to activate a session. The player enters normally, opens ESC, and selects **Admin Panel**. After successful activation, the button changes to **Logout from Admin Panel**. Logging out removes the granted GM role and releases its slot.

Each staff entry has independent account-enabled, GM, News, Players, Kick, Ban, and limit-bypass switches. Rank is a descriptive label. The switches determine the actual abilities. An in-game news broadcast requires an active administration session and News permission.

The ordinary GM limit can be set from one to three, with three as the default. Staff with **Bypass Limit** do not consume ordinary slots. A news-only administrator can activate a session without taking a GM slot. The server checks permissions when processing actions and reloads configuration when its revision changes.

Revoking permissions or disabling an account ends sessions that no longer meet their requirements. Other scenarios or addons may grant GM access through their own systems, so the server owner needs to coordinate systems that control the same role.

## News commands

Send the command and complete message in one chat entry. Duration is optional and defaults to **60 seconds**. Supported durations are **15–180 seconds**, and messages contain **1–600 characters** on one line.

| Command | Audience and displayed sender |
| --- | --- |
| `#news 20 message` | The sender's own faction, using its sender label and portrait |
| `#news all 20 message` | Everyone, using the sender's faction label and portrait |
| `#news server 20 message` | Everyone, identified as a server announcement |
| `#news us 20 message` | The US faction group |
| `#news ussr 20 message` | The USSR faction |
| `#news stop` | Stops the current broadcast for everyone |

Examples

```text
#news 20 The coordination meeting starts in two minutes
#news all 30 Announcement to all players from my faction
#news server 20 Official server administration announcement
#news us 20 Announcement for the US faction
#news ussr 20 Announcement for the USSR faction
#news stop
```

The default audience matches the sender's faction key exactly. This mode is called `self` internally. Use the command without an audience selector to reach your faction. The sender needs a selected faction for this default mode. Use `server` for a general announcement when no faction is selected.

The `us` selector matches faction keys `US`, `USA`, and `ARMY`. The `ussr` selector matches the exact faction key `USSR`. Delivery depends on the faction keys used by the scenario. Audience selectors are case-insensitive. Enter the stop command exactly as `#news stop`.

The ticker starts with the beginning of the message and adjusts its speed to the text length and display duration. Broadcasts have a **three-second cooldown**. A subsequent broadcast sent during that interval may be rejected. The chat response reports delivery or the reason for rejection.

The Windows News page sends an official announcement with the shared BLC logo and the prefix **Server Admin says:**. Select everyone, US only, or USSR only and choose the duration. Follow the command ID and its server result to confirm execution.

GM properties include **News Sound (On / Off)**. The setting controls news audio, and switching it off stops the current news sound. The bundled audio level has been reduced.

## Players, kicks, and bans

The manager's Players page combines connected players and historical records. It includes name, UUID, platform, first and last seen times, and aliases. The operator can appoint staff or ban a historical player, kick a connected player, or end their administration session.

The in-game **Admin Players** window becomes available when the session is active and player-list permission is granted. Kick and ban controls follow their respective permissions. Enter a reason and a ban duration in minutes. **0** means permanent. The in-game maximum is **43,200 minutes**.

In Game Master, select one player or player icon and open the action menu. **BLCA | Kick Player** and **BLCA | Ban Player** open a form for the reason and, for a ban, its duration. These actions require an active session plus GM, player-list, and the corresponding moderation permission.

Temporary bans are stored with an absolute UTC expiry. The addon stops enforcing them when that time is reached, even if the Windows application has closed. The manager displays times in the computer's local timezone. A ban requested from inside the game requires the manager or background agent to process and save the request. The audit records show its outcome.

A banned player receives a notice with instructions to contact server administration, the reason, and the expiry or permanent-ban label. The server allows **10 seconds** after sending the notice before attempting a kick. Closing the notice does not cancel the deadline. If the notice cannot be delivered, a **20-second** fallback deadline starts when the ban is detected. The server retries the kick every two seconds while the connection remains, and cancels enforcement if the ban expires or is lifted during the wait.

The notice is delivered after the connection identity is authenticated. The game's native disconnect dialog may display a generic message, and notice details may not arrive if the client disconnects early. Bans stored in another system must be managed through their source.

## Weather controls

Open **Game Weather** in the Windows manager, choose a preset reported by the scenario, and select **Apply Weather**. The server processes the request. The page displays the effective current weather and the command result. **Automatic by scenario** resumes the original weather progression.

The available presets come from the current game world. Controls are disabled while the addon is offline, weather status is missing, or the world has no supported weather manager. Scenario events may change the weather later, and subsequent status updates show the effective state.

## Audit and activity reports

The addon records player joins and leaves, administration sessions, GM opening, news, kicks, bans, and manager command results. Daily NDJSON audit files are stored in `audit`. The Operations page displays the latest **800 events** and can export that view to CSV.

To include GM operations written by the engine, such as spawning entities, select the server's `logs` folder in Settings. The application reads `script.log` and associates entries containing a UUID with their actor. The available detail depends on what the engine records.

Reports cover administrator activity and player connection hours for a selected day, week, or the complete archive. Weeks begin on **Monday**, and calendar boundaries use the timezone of the computer running the manager. Playtime is displayed as hours, minutes, and seconds. Reports use the full available audit archive.

Administrator columns include session logins, GM opening, GM operations, news, kicks, bans, and other successful actions. Windows operator activity is shown separately. Totals exclude queued-only and rejected operations and avoid counting editor-open and editor-close messages as extra GM operations.

Connection hours are calculated from join/leave pairs and a fresh live status for currently connected players. An old join without a leave or current heartbeat cannot establish continuing playtime. The page indicates when the available archive begins partway through the requested reporting period.

UUID is the persistent identity used for permissions and history. The temporary network-session identity is a separate field. IP address, country, platform account ID, and external GUID may require an additional trusted data source. The manager supports UUID-keyed CSV metadata import, while unavailable fields remain empty.

## Background agent

To continue importing GM logs and processing in-game ban requests after closing the main window, run the manager in agent mode

```powershell
& ".\BLCA Admin Manger.exe" --agent --profile "D:\Reforger\ServerA\profile" --logs "D:\Reforger\ServerA\logs"
```

Use your server's actual paths. The agent provides a tray icon with an exit action. One archive-processing process is sufficient for a server profile. Permission enforcement, ban expiry, and in-game news delivery run inside the addon.

The command queue holds **32 requests** and the addon processes up to **four requests** per update cycle. Requests expire after **120 seconds**. The manager displays addon update timings and dropped audit-event counts for monitoring actual load.

## Test world and Workbench

The project includes a Game Master test world on Arland. Its mission resource is `{C716458FF9B84A05}Missions/BLCA_NewsTest.conf`. Open it in Workbench or select the test mission after loading the addon.

During a Workbench play session, enter `#newmenu` to open the pause menu while ESC is used to return to the editor. Manual news previews display locally for the command's player and require no administration permission

```text
#newstest 20 Local news ticker preview
#newstest ussr 20 USSR sender and portrait preview
#newstest server 20 Server announcement preview
#newstest stop
```

Previews start when their commands are entered. To check actual faction delivery, use `#news` with an active staff session and News permission, then verify the result on the intended recipients.

## Data files and troubleshooting

| File or folder | Purpose |
| --- | --- |
| `config.json` and `config.rev` | Staff, bans, and configuration revision |
| `status.json` | Addon connection, players, weather, and command results |
| `commands.json` and `commands.rev` | Windows command queue |
| `command_cursor.txt` | Last command accepted by the addon |
| `ban-requests.json` | In-game ban requests |
| `audit` | Operations archive |
| `manager-data` | Player history, archive cursors, and manager data |

A missing-permission response to news indicates the need to activate Admin Panel or grant News. A full-GM-slots response indicates the ordinary slot limit. If the manager is waiting for the addon, check the selected profile and confirm that `status.json` is being updated inside that same folder.

Compatibility depends on the current game and scenario APIs. The project's multiple platform configurations do not establish hardware testing on every platform. Included `Validation` files describe the revisions they reference and do not establish server-specific performance or a new test result for this visual rebrand.

**BLCA Development Team**
