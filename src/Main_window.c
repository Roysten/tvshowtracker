#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <synchapi.h>
#include <time.h>

#include <windef.h>
#include <winuser.h>
#include <commctrl.h>

#include "App.h"
#include "dialog/About_dialog.h"
#include "Globals.h"
#include "Logger.h"
#include "Main_window.h"
#include "Resource.h"
#include "Tv_show.h"
#include "ui/Button.h"
#include "ui/Groupbox.h"
#include "ui/Listbox.h"
#include "ui/Listview.h"
#include "ui/Statusbar.h"
#include "ui/Textbox.h"
#include "Requester_thread.h"
#include "util/vec.h"

/* Main window class and title */
static const wchar_t main_window_class[] = L"TV Show Tracker";
static const char APP_STATE_PATH[] = "app_state.json";

static wchar_t *LISTVIEW_COL_NAMES[] = {
	L"Ep",
	L"Air date",
	L"Title",
};

static const int WINDOW_W = 640;
static const int WINDOW_H = 640;

static const int PADDING = 10;

static const int TEXTBOX_H = 20;
static const int BUTTON_H = 22;

static const int LISTVIEW_SHOWS_W = 150;
static const int LISTBOX_SEARCH_SUGGESTIONS_H = 120;
static const int GROUPBOX_SEARCH_H = LISTBOX_SEARCH_SUGGESTIONS_H + TEXTBOX_H + BUTTON_H + 3 * PADDING;

static const int TEXTBOX_SEARCH_SHOW_MAX_TEXT_LEN = 250;

static const int BUTTON_ADD_W = 60;

static Groupbox groupbox;
static Statusbar statusbar;
static Textbox textbox_search_show;
static Listview listview_shows;
static Listview listview_episodes;
static Listbox listbox_search_suggestions;
static Button button_add;

static App app;
static Tv_show search_suggestions[20];

static Requester_thread requester_thread;

static wchar_t search_buf[256];

static LRESULT CALLBACK window_event_loop(HWND hWnd, UINT msg, WPARAM w_param, LPARAM l_param);
static void create_gui(HWND hwnd);
static void init_gui_from_app(void);
static void on_textbox_submit(Textbox *textbox);
static void on_search_completed(bool success);
static void on_search_suggestion_selected(Listbox *listbox);
static void on_tv_show_add(void);
static void on_tv_show_info_loaded_add(bool success, Tv_show *show);
static void on_window_exit(void);
static void on_listview_show_requested(Listview *listview, LVITEMW *item);
static void on_listview_episode_requested(Listview *listview, LVITEMW *item);
static void on_tv_show_selected(Listview *listview);
static unsigned on_listview_episodes_draw_item(LPNMLVCUSTOMDRAW custom_draw);
static void on_tv_show_refresh(void);
static void on_tv_show_refreshed(bool success, Tv_show * show);
static void on_tv_shows_refresh(void);
static void on_tv_shows_refreshed(void);
static void set_controls_enabled(bool enabled);

/* Register a class for our main window */
BOOL RegisterMainWindowClass()
{
	WNDCLASSEX wc = {0};

	/* Class for our main window */
	wc.cbSize        = sizeof(wc);
	wc.style         = 0;
	wc.lpfnWndProc   = &window_event_loop;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = g_hInstance;
	wc.hIcon         = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE |
										LR_DEFAULTCOLOR | LR_SHARED);
	wc.hCursor       = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = main_window_class;
	wc.hIconSm       = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	return (RegisterClassEx(&wc)) ? TRUE : FALSE;
}

/* Create an instance of our main window */
HWND CreateMainWindow()
{
	/* Create instance of main window */
	HWND hWnd = CreateWindowEx(0, main_window_class, main_window_class, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_W, WINDOW_H, NULL, NULL, g_hInstance, NULL);
	return hWnd;
}

static LRESULT CALLBACK window_event_loop(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
	switch (msg)
	{
		case WM_CREATE: {
			create_gui(hwnd);
			REQUESTER_THREAD_create(&requester_thread);
			APP_from_file(&app, APP_STATE_PATH);
			init_gui_from_app();
			break;
		}
		case WM_SIZE:
			STATUSBAR_on_resize(&statusbar);
			break;
		case WM_NOTIFY: {
			WORD id = LOWORD(w_param);	
			unsigned code = ((LPNMHDR) l_param)->code;
			if (id == ID_LISTVIEW_EPISODES) {
				if (code == NM_CUSTOMDRAW) {
					LPNMLVCUSTOMDRAW custom_draw = (LPNMLVCUSTOMDRAW) l_param;
					return on_listview_episodes_draw_item(custom_draw);
				}

				if (code == LVN_GETDISPINFO) {
					NMLVDISPINFOW *info = (NMLVDISPINFOW *) l_param;
					on_listview_episode_requested(&listview_episodes, &info->item);
					return TRUE;
				}
			} else if (id == ID_LISTVIEW_SHOWS) {
				if (code == LVN_GETDISPINFO) {
					NMLVDISPINFOW *info = (NMLVDISPINFOW *) l_param;
					on_listview_show_requested(&listview_shows, &info->item);
					return TRUE;
				}

				if (code == LVN_ITEMCHANGED) {
					// LPNMLISTVIEW info = (LPNMLISTVIEW) l_param;
					// if (info->iItem == -1 || (info->iItem >= 0 && (info->uNewState & LVIS_SELECTED) != 0 && (info->uOldState & LVIS_SELECTED) == 0)) {
						on_tv_show_selected(&listview_shows);
					// }
					return 0;
				}

				if (code == LVN_GETINFOTIP) {
					LPNMLVGETINFOTIPW tooltip = (LPNMLVGETINFOTIPW) l_param;
					wchar_t date_buf[256];
					struct tm tm;
					gmtime_s(&tm, &vec_get(Tv_show, app.tv_shows, tooltip->iItem)->last_sync);
					wcsftime(date_buf, tooltip->cchTextMax, L"%Y-%m-%d %H:%M UTC", &tm);
					wsprintf(tooltip->pszText, L"Last synchronized: %ls", date_buf);
				}
			}
			break;
		}
		case WM_COMMAND: {
			WORD id = LOWORD(w_param);

			switch (id)
			{
				case ID_HELP_ABOUT:
					ShowAboutDialog(hwnd);
					return 0;
				case ID_FILE_REFRESH_ALL:
					on_tv_shows_refresh();
					return 0;
				case ID_FILE_EXIT:
					DestroyWindow(hwnd);
					return 0;
				case ID_LISTBOX_SEARCH_SUGGESTIONS:
					if (HIWORD(w_param) == LBN_SELCHANGE) {
						on_search_suggestion_selected(&listbox_search_suggestions);
					}
					return 0;
				case ID_LISTVIEW_SHOWS:
					if(HIWORD(w_param) == LBN_SELCHANGE) {
						on_tv_show_selected(&listview_shows);
					}
					break;
				case ID_BUTTON_ADD:
					on_tv_show_add();
					break;
				case ID_MENU_REFRESH_SHOW:
					on_tv_show_refresh();
					break;
			}
			break;
		}
		case WM_CONTEXTMENU: {
			if ((HWND) w_param == listview_shows.control) {
				if (LISTVIEW_get_selected_index(&listview_shows) == -1) {
					return 0;
				}

				HMENU menu = CreatePopupMenu();
				AppendMenu(menu, MF_STRING, ID_MENU_REFRESH_SHOW, L"Refresh");
				TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN, LOWORD(l_param), HIWORD(l_param), 0, hwnd, NULL);
				DestroyMenu(menu);
				break;
			}
			return 0;
		}
		case WM_GETMINMAXINFO: {
			/* Prevent our window from being sized too small */
			MINMAXINFO *minMax = (MINMAXINFO*)l_param;
			minMax->ptMinTrackSize.x = WINDOW_W;
			minMax->ptMinTrackSize.y = WINDOW_H;

			return 0;
		}
		case WM_DESTROY:
		{
			on_window_exit();
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, w_param, l_param);
}

static void create_gui(HWND hwnd)
{
	STATUSBAR_create(&statusbar, hwnd, (HMENU) ID_STATUSBAR);

	RECT rect;
	GetClientRect(hwnd, &rect);
	int max_w = rect.right - rect.left;
	int max_h = rect.bottom - rect.top;

	int offset_y = PADDING;
	LISTVIEW_create(&listview_shows, hwnd, (HMENU) ID_LISTVIEW_SHOWS, PADDING, offset_y, LISTVIEW_SHOWS_W, 332);
	LISTVIEW_add_column(&listview_shows, L"Name", 0);

	LISTVIEW_create(&listview_episodes, hwnd, (HMENU) ID_LISTVIEW_EPISODES, LISTVIEW_SHOWS_W + 2 * PADDING, offset_y, max_w - 4 * PADDING - LISTVIEW_SHOWS_W, 332);
	LISTVIEW_set_enabled(&listview_episodes, false);
	LISTVIEW_set_groups_enabled(&listview_episodes, true);
	for (int i = 0; i < ARRAYSIZE(LISTVIEW_COL_NAMES); ++i) {
		LISTVIEW_add_column(&listview_episodes, LISTVIEW_COL_NAMES[i], i);
	}

	offset_y = max_h - 230;
	GROUPBOX_create(&groupbox, hwnd, (HMENU) ID_GROUPBOX_SEARCH, L"Search", PADDING, offset_y, max_w - 2 * PADDING, GROUPBOX_SEARCH_H);
	offset_y += PADDING * 2;

	TEXTBOX_create(&textbox_search_show, hwnd, (HMENU) ID_TEXTBOX_SEARCH_SHOW, PADDING * 2, offset_y, max_w - 4 * PADDING, TEXTBOX_H);
	TEXTBOX_set_text_limit(&textbox_search_show, TEXTBOX_SEARCH_SHOW_MAX_TEXT_LEN);
	TEXTBOX_set_placeholder_text(&textbox_search_show, L"Name of TV show");
	TEXTBOX_set_on_submit(&textbox_search_show, on_textbox_submit);
	offset_y += PADDING + TEXTBOX_H;

	LISTBOX_create(&listbox_search_suggestions, hwnd, (HMENU) ID_LISTBOX_SEARCH_SUGGESTIONS, PADDING * 2, offset_y, max_w - 4 * PADDING, LISTBOX_SEARCH_SUGGESTIONS_H);
	LISTBOX_set_enabled(&listbox_search_suggestions, false);
	offset_y += LISTBOX_SEARCH_SUGGESTIONS_H - PADDING;

	BUTTON_create(&button_add, hwnd, (HMENU) ID_BUTTON_ADD, L"Add", PADDING * 2, offset_y, BUTTON_ADD_W, BUTTON_H);
	BUTTON_set_enabled(&button_add, false);
}

static void init_gui_from_app(void)
{
	LISTVIEW_set_item_count(&listview_shows, vec_size(app.tv_shows));
	LISTVIEW_column_auto_width_header(&listview_shows, 0);
}

static void on_textbox_submit(Textbox *textbox)
{
	if (textbox == &textbox_search_show) {
		TEXTBOX_get_text(textbox, search_buf, TEXTBOX_SEARCH_SHOW_MAX_TEXT_LEN + 1);
		TEXTBOX_set_enabled(textbox, false);

		LOG("Textbox text: %ls", search_buf);

		STATUSBAR_set_text(&statusbar, L"Search in progress");
		REQUESTER_THREAD_search_show(&requester_thread, search_buf, search_suggestions, ARRAYSIZE(search_suggestions), on_search_completed);
	}
}

static void on_search_completed(bool success)
{
	STATUSBAR_set_text(&statusbar, NULL);
	if (!success) {
		MessageBoxW(NULL, L"Search failed, make sure you have a working internet connection.", L"Search error", MB_OK | MB_ICONERROR);
	}

	LISTBOX_reset_content(&listbox_search_suggestions);
	for (size_t i = 0; i < ARRAYSIZE(search_suggestions) && search_suggestions[i].id != 0; ++i) {
		LISTBOX_add_string(&listbox_search_suggestions, search_suggestions[i].name);
	}

	BUTTON_set_enabled(&button_add, false);
	LISTBOX_set_enabled(&listbox_search_suggestions, true);
	TEXTBOX_set_enabled(&textbox_search_show, true);
}

static void on_search_suggestion_selected(Listbox *listbox)
{
	BUTTON_set_enabled(&button_add, true);
}

static void on_tv_show_add(void)
{
	DWORD selected_index = LISTBOX_get_selected_index(&listbox_search_suggestions);
	if (selected_index == LB_ERR) {
		return;
	}
	set_controls_enabled(false);
	REQUESTER_THREAD_get_show_info(&requester_thread, search_suggestions[selected_index].id, on_tv_show_info_loaded_add);
}

static void on_tv_show_info_loaded_add(bool success, Tv_show *show)
{
	set_controls_enabled(true);
	if (!success) {
		LOG("TV show could not be loaded!");
		return;
	}

	vec_push(Tv_show, app.tv_shows, *show);
	free(show);
	LISTVIEW_set_item_count(&listview_shows, vec_size(app.tv_shows));
	LISTVIEW_column_auto_width_header(&listview_shows, 0);
	app.dirty = true;
}

static void on_window_exit(void)
{
	if (app.dirty) {
		APP_to_file(&app, APP_STATE_PATH);
	}
}

static void on_listview_show_requested(Listview *listview, LVITEMW *item)
{
	if ((item->mask & LVIF_TEXT) == 0 || item->iItem >= vec_size(app.tv_shows)) {
		return;
	}

	Tv_show *show = vec_get(Tv_show, app.tv_shows, item->iItem);
	wsprintf(item->pszText, L"%s", show->name);
}


static void on_listview_episode_requested(Listview *listview, LVITEMW *item)
{
	if ((item->mask & LVIF_TEXT) == 0 && (item->mask & LVIF_GROUPID) == 0) {
		return;
	}
	
	int selected_show_index = LISTVIEW_get_selected_index(&listview_shows);
	if (selected_show_index == -1) {
		return;
	}

	Tv_show *show = vec_get(Tv_show, app.tv_shows, selected_show_index);
	if (item->iItem < vec_size(show->episodes)) {
		Tv_show_episode *episode = vec_get(Tv_show_episode, show->episodes, item->iItem);
		item->iGroupId = episode->season_number;

		if ((item->mask & LVIF_TEXT) != 0) {
			switch (item->iSubItem) {
				case 0:
					wsprintf(item->pszText, L"%d", episode->season_episode_number);
					break;
				case 1: {
					struct tm tm;
					gmtime_s(&tm, &episode->air_date);
					wcsftime(item->pszText, item->cchTextMax, L"%Y-%m-%d", &tm);
					break;
				}
				case 2:
					item->pszText = (wchar_t*) episode->name;
					break;
			}
		}		
	}
}

static void on_tv_show_selected(Listview *listview)
{
	int index = LISTVIEW_get_selected_index(listview);
	if (index >= 0 && index < vec_size(app.tv_shows)) {
		LISTVIEW_clear_items(&listview_episodes);			
		LISTVIEW_clear_groups(&listview_episodes);			
		LISTVIEW_set_enabled(&listview_episodes, true);

		Tv_show *show = vec_get(Tv_show, app.tv_shows, index);
		int episode_count = vec_size(show->episodes);
		for (int i = 0; i < vec_size(show->seasons); ++i) {
			Tv_show_season *season = vec_get(Tv_show_season, show->seasons, i);
			LISTVIEW_add_group(&listview_episodes, (wchar_t *) season->name, season->season_number);
		}
		LISTVIEW_set_item_count(&listview_episodes, episode_count);

		time_t now = time(NULL);
		for (int i = 0; i < vec_size(show->episodes); ++i) {
			Tv_show_episode *episode = vec_get(Tv_show_episode, show->episodes, i);
			if (episode->air_date > now) {
				LVITEM item = (LVITEM) {0};
				item.iItem = i;
				item.mask = LVIF_PARAM;
				if (ListView_GetItem(listview_episodes.control, &item)) {
					item.lParam = 1;
					ListView_SetItem(listview_episodes.control, &item);
				}

				LISTVIEW_set_collapsed(&listview_episodes, episode->season_number, false);
				break;
			}
		}
		SetFocus(listview_episodes.control);
	} else {
		LISTVIEW_clear_items(&listview_episodes);			
		LISTVIEW_clear_groups(&listview_episodes);			
		LISTVIEW_set_enabled(&listview_episodes, false);
	}

	int i = 0;
	for (; i < ARRAYSIZE(LISTVIEW_COL_NAMES) - 1; ++i) {
		LISTVIEW_column_auto_width(&listview_episodes, i - 1);
	}
	LISTVIEW_column_auto_width_header(&listview_episodes, i);
}

unsigned on_listview_episodes_draw_item(LPNMLVCUSTOMDRAW custom_draw)
{
	switch(custom_draw->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT: {
			if (custom_draw->nmcd.lItemlParam != 0) {
				custom_draw->clrText = GetSysColor(COLOR_HIGHLIGHT);
			}
			return CDRF_NEWFONT;
		}
	}

	return CDRF_DODEFAULT;
}

void on_tv_show_refresh(void) {
	Tv_show *show = vec_get(Tv_show, app.tv_shows, LISTVIEW_get_selected_index(&listview_shows));
	LOG("Refreshing show: %d", show->id);
	REQUESTER_THREAD_get_show_info(&requester_thread, show->id, on_tv_show_refreshed);
}

void on_tv_show_refreshed(bool success, Tv_show *show)
{
	if (!success) {
		LOG("Refreshing show failed");
		TV_SHOW_destroy(show);
		return;
	}

	size_t show_to_replace_index = 0;
	Tv_show *show_to_replace = NULL;
	for (size_t i = 0; i < vec_size(app.tv_shows); ++i) {
		Tv_show *s = vec_get(Tv_show, app.tv_shows, i);
		if (show->id == s->id) {
			show_to_replace = s;
			break;
		}
	}

	if (show_to_replace == NULL) {
		LOG("Refreshed show not found");
		TV_SHOW_destroy(show);
		return;
	}

	TV_SHOW_destroy(show_to_replace);
	*show_to_replace = *show;
	app.dirty = true;
	ListView_RedrawItems(listview_shows.control, show_to_replace_index, show_to_replace_index);
	on_tv_show_selected(&listview_shows);
}

static void on_tv_shows_refresh(void)
{
	set_controls_enabled(false);
	REQUESTER_THREAD_refresh_shows(&requester_thread, app.tv_shows, on_tv_show_refreshed, on_tv_shows_refreshed);
}

static void on_tv_shows_refreshed(void)
{
	set_controls_enabled(true);
	LOG("Shows refreshed!");
}

static void set_controls_enabled(bool enabled)
{
	EnableWindow(listview_shows.control, enabled);
	EnableWindow(listview_episodes.control, enabled);
	EnableWindow(listbox_search_suggestions.control, enabled);
	EnableWindow(textbox_search_show.control, enabled);
	EnableWindow(button_add.control, enabled);
}
