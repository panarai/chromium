// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dom_ui/browser_options_handler.h"

#include "app/l10n_util.h"
#include "base/basictypes.h"
#include "base/scoped_ptr.h"
#include "base/string_number_conversions.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/custom_home_pages_table_model.h"
#include "chrome/browser/profile.h"
#include "chrome/browser/session_startup_pref.h"
#include "chrome/installer/util/browser_distribution.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"

BrowserOptionsHandler::BrowserOptionsHandler()
    : template_url_model_(NULL), startup_custom_pages_table_model_(NULL) {
#if !defined(OS_MACOSX)
  default_browser_worker_ = new ShellIntegration::DefaultBrowserWorker(this);
#endif
}

BrowserOptionsHandler::~BrowserOptionsHandler() {
  if (default_browser_worker_.get())
    default_browser_worker_->ObserverDestroyed();
  if (template_url_model_)
    template_url_model_->RemoveObserver(this);
}

void BrowserOptionsHandler::GetLocalizedValues(
    DictionaryValue* localized_strings) {
  DCHECK(localized_strings);
  localized_strings->SetString("startupGroupName",
      l10n_util::GetStringUTF16(IDS_OPTIONS_STARTUP_GROUP_NAME));
  localized_strings->SetString("startupShowDefaultAndNewTab",
      l10n_util::GetStringUTF16(IDS_OPTIONS_STARTUP_SHOW_DEFAULT_AND_NEWTAB));
  localized_strings->SetString("startupShowLastSession",
      l10n_util::GetStringUTF16(IDS_OPTIONS_STARTUP_SHOW_LAST_SESSION));
  localized_strings->SetString("startupShowPages",
      l10n_util::GetStringUTF16(IDS_OPTIONS_STARTUP_SHOW_PAGES));
  localized_strings->SetString("startupAddButton",
      l10n_util::GetStringUTF16(IDS_OPTIONS_STARTUP_ADD_BUTTON));
  localized_strings->SetString("startupRemoveButton",
      l10n_util::GetStringUTF16(IDS_OPTIONS_STARTUP_REMOVE_BUTTON));
  localized_strings->SetString("startupUseCurrent",
      l10n_util::GetStringUTF16(IDS_OPTIONS_STARTUP_USE_CURRENT));
  localized_strings->SetString("homepageGroupName",
      l10n_util::GetStringUTF16(IDS_OPTIONS_HOMEPAGE_GROUP_NAME));
  localized_strings->SetString("homepageUseNewTab",
      l10n_util::GetStringUTF16(IDS_OPTIONS_HOMEPAGE_USE_NEWTAB));
  localized_strings->SetString("homepageUseURL",
      l10n_util::GetStringUTF16(IDS_OPTIONS_HOMEPAGE_USE_URL));
  localized_strings->SetString("homepageShowButton",
      l10n_util::GetStringUTF16(IDS_OPTIONS_HOMEPAGE_SHOW_BUTTON));
  localized_strings->SetString("defaultSearchGroupName",
      l10n_util::GetStringUTF16(IDS_OPTIONS_DEFAULTSEARCH_GROUP_NAME));
  localized_strings->SetString("defaultSearchManageEnginesLink",
      l10n_util::GetStringUTF16(IDS_OPTIONS_DEFAULTSEARCH_MANAGE_ENGINES_LINK));
  localized_strings->SetString("defaultBrowserGroupName",
      l10n_util::GetStringUTF16(IDS_OPTIONS_DEFAULTBROWSER_GROUP_NAME));
  localized_strings->SetString("defaultBrowserUnknown",
      l10n_util::GetStringFUTF16(IDS_OPTIONS_DEFAULTBROWSER_UNKNOWN,
          l10n_util::GetStringUTF16(IDS_PRODUCT_NAME)));
  localized_strings->SetString("defaultBrowserUseAsDefault",
      l10n_util::GetStringFUTF16(IDS_OPTIONS_DEFAULTBROWSER_USEASDEFAULT,
          l10n_util::GetStringUTF16(IDS_PRODUCT_NAME)));
}

void BrowserOptionsHandler::RegisterMessages() {
  dom_ui_->RegisterMessageCallback(
      "becomeDefaultBrowser",
      NewCallback(this, &BrowserOptionsHandler::BecomeDefaultBrowser));
  dom_ui_->RegisterMessageCallback(
      "setDefaultSearchEngine",
      NewCallback(this, &BrowserOptionsHandler::SetDefaultSearchEngine));
  dom_ui_->RegisterMessageCallback(
      "removeStartupPages",
      NewCallback(this, &BrowserOptionsHandler::RemoveStartupPages));
  dom_ui_->RegisterMessageCallback(
      "setStartupPagesToCurrentPages",
      NewCallback(this, &BrowserOptionsHandler::SetStartupPagesToCurrentPages));
}

void BrowserOptionsHandler::Initialize() {
  UpdateDefaultBrowserState();
  UpdateStartupPages();
  UpdateSearchEngines();
}

void BrowserOptionsHandler::UpdateDefaultBrowserState() {
#if defined(OS_WIN)
  // Check for side-by-side first.
  if (!BrowserDistribution::GetDistribution()->CanSetAsDefault()) {
    SetDefaultBrowserUIString(IDS_OPTIONS_DEFAULTBROWSER_SXS);
    return;
  }
#endif

#if defined(OS_MACOSX)
  ShellIntegration::DefaultBrowserState state =
      ShellIntegration::IsDefaultBrowser();
  int status_string_id;
  if (state == ShellIntegration::IS_DEFAULT_BROWSER)
    status_string_id = IDS_OPTIONS_DEFAULTBROWSER_DEFAULT;
  else if (state == ShellIntegration::NOT_DEFAULT_BROWSER)
    status_string_id = IDS_OPTIONS_DEFAULTBROWSER_NOTDEFAULT;
  else
    status_string_id = IDS_OPTIONS_DEFAULTBROWSER_UNKNOWN;

  SetDefaultBrowserUIString(status_string_id);
#else
  default_browser_worker_->StartCheckDefaultBrowser();
#endif
}

void BrowserOptionsHandler::BecomeDefaultBrowser(const Value* value) {
#if defined(OS_MACOSX)
  if (ShellIntegration::SetAsDefaultBrowser())
    UpdateDefaultBrowserState();
#else
  default_browser_worker_->StartSetAsDefaultBrowser();
  // Callback takes care of updating UI.
#endif
}

int BrowserOptionsHandler::StatusStringIdForState(
    ShellIntegration::DefaultBrowserState state) {
  if (state == ShellIntegration::IS_DEFAULT_BROWSER)
    return IDS_OPTIONS_DEFAULTBROWSER_DEFAULT;
  if (state == ShellIntegration::NOT_DEFAULT_BROWSER)
    return IDS_OPTIONS_DEFAULTBROWSER_NOTDEFAULT;
  return IDS_OPTIONS_DEFAULTBROWSER_UNKNOWN;
}

void BrowserOptionsHandler::SetDefaultBrowserUIState(
    ShellIntegration::DefaultBrowserUIState state) {
  int status_string_id;
  if (state == ShellIntegration::STATE_IS_DEFAULT)
    status_string_id = IDS_OPTIONS_DEFAULTBROWSER_DEFAULT;
  else if (state == ShellIntegration::STATE_NOT_DEFAULT)
    status_string_id = IDS_OPTIONS_DEFAULTBROWSER_NOTDEFAULT;
  else if (state == ShellIntegration::STATE_UNKNOWN)
    status_string_id = IDS_OPTIONS_DEFAULTBROWSER_UNKNOWN;
  else
    return;  // Still processing.

  SetDefaultBrowserUIString(status_string_id);
}

void BrowserOptionsHandler::SetDefaultBrowserUIString(int status_string_id) {
  scoped_ptr<Value> status_string(Value::CreateStringValue(
      l10n_util::GetStringFUTF16(status_string_id,
                                 l10n_util::GetStringUTF16(IDS_PRODUCT_NAME))));

  scoped_ptr<Value> is_default(Value::CreateBooleanValue(
      status_string_id == IDS_OPTIONS_DEFAULTBROWSER_DEFAULT));

  dom_ui_->CallJavascriptFunction(
      L"BrowserOptions.updateDefaultBrowserState",
      *(status_string.get()), *(is_default.get()));
}

void BrowserOptionsHandler::OnTemplateURLModelChanged() {
  if (!template_url_model_ || !template_url_model_->loaded())
    return;

  const TemplateURL* default_url =
      template_url_model_->GetDefaultSearchProvider();

  int default_index = 0;
  ListValue search_engines;
  std::vector<const TemplateURL*> model_urls =
      template_url_model_->GetTemplateURLs();
  for (size_t i = 0; i < model_urls.size(); ++i) {
    if (!model_urls[i]->ShowInDefaultList())
      continue;

    DictionaryValue* entry = new DictionaryValue();
    entry->SetString("name", WideToUTF16Hack(model_urls[i]->short_name()));
    entry->SetInteger("index", i);
    search_engines.Append(entry);
    if (model_urls[i] == default_url)
      default_index = i;
  }

  scoped_ptr<Value> default_value(Value::CreateIntegerValue(default_index));

  dom_ui_->CallJavascriptFunction(L"BrowserOptions.updateSearchEngines",
                                  search_engines, *(default_value.get()));
}

void BrowserOptionsHandler::SetDefaultSearchEngine(const Value* value) {
  if (!value || !value->IsType(Value::TYPE_LIST)) {
    NOTREACHED();
    return;
  }
  const ListValue* param_values = static_cast<const ListValue*>(value);
  std::string string_value;
  if (param_values->GetSize() != 1 ||
      !param_values->GetString(0, &string_value)) {
    NOTREACHED();
    return;
  }
  int selected_index;
  base::StringToInt(string_value, &selected_index);

  std::vector<const TemplateURL*> model_urls =
      template_url_model_->GetTemplateURLs();
  if (selected_index >= 0 &&
      selected_index < static_cast<int>(model_urls.size()))
    template_url_model_->SetDefaultSearchProvider(model_urls[selected_index]);
}

void BrowserOptionsHandler::UpdateSearchEngines() {
  template_url_model_ = dom_ui_->GetProfile()->GetTemplateURLModel();
  if (template_url_model_) {
    template_url_model_->Load();
    template_url_model_->AddObserver(this);
    OnTemplateURLModelChanged();
  }
}

void BrowserOptionsHandler::UpdateStartupPages() {
  Profile* profile = dom_ui_->GetProfile();
  startup_custom_pages_table_model_.reset(
      new CustomHomePagesTableModel(profile));
  startup_custom_pages_table_model_->SetObserver(this);

  const SessionStartupPref startup_pref =
      SessionStartupPref::GetStartupPref(profile->GetPrefs());
  startup_custom_pages_table_model_->SetURLs(startup_pref.urls);
}

void BrowserOptionsHandler::OnModelChanged() {
  // TODO(stuartmorgan): Add support for showing favicons.
  ListValue startup_pages;
  int page_count = startup_custom_pages_table_model_->RowCount();
  for (int i = 0; i < page_count; ++i) {
    DictionaryValue* entry = new DictionaryValue();
    entry->SetString("title", WideToUTF16Hack(
                        startup_custom_pages_table_model_->GetText(i, 0)));
    entry->SetString("tooltip", WideToUTF16Hack(
                         startup_custom_pages_table_model_->GetTooltip(i)));
    startup_pages.Append(entry);
  }

  dom_ui_->CallJavascriptFunction(L"BrowserOptions.updateStartupPages",
                                  startup_pages);
}

void BrowserOptionsHandler::OnItemsChanged(int start, int length) {
  OnModelChanged();
}

void BrowserOptionsHandler::OnItemsAdded(int start, int length) {
  OnModelChanged();
}

void BrowserOptionsHandler::OnItemsRemoved(int start, int length) {
  OnModelChanged();
}

void BrowserOptionsHandler::SetStartupPagesToCurrentPages(const Value* value) {
  startup_custom_pages_table_model_->SetToCurrentlyOpenPages();
  SaveStartupPagesPref();
}

void BrowserOptionsHandler::RemoveStartupPages(const Value* value) {
  if (!value || !value->IsType(Value::TYPE_LIST)) {
    NOTREACHED();
    return;
  }
  const ListValue* param_values = static_cast<const ListValue*>(value);
  for (int i = param_values->GetSize() - 1; i >= 0; --i) {
    std::string string_value;
    if (!param_values->GetString(i, &string_value)) {
      NOTREACHED();
      return;
    }
    int selected_index;
    base::StringToInt(string_value, &selected_index);
    if (selected_index < 0 ||
        selected_index >= startup_custom_pages_table_model_->RowCount()) {
      NOTREACHED();
      return;
    }
    startup_custom_pages_table_model_->Remove(selected_index);
  }

  SaveStartupPagesPref();
}

void BrowserOptionsHandler::SaveStartupPagesPref() {
  PrefService* prefs = dom_ui_->GetProfile()->GetPrefs();

  SessionStartupPref pref = SessionStartupPref::GetStartupPref(prefs);
  pref.urls = startup_custom_pages_table_model_->GetURLs();

  SessionStartupPref::SetStartupPref(prefs, pref);
}
