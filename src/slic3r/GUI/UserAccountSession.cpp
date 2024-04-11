#include "UserAccountSession.hpp"
#include "GUI_App.hpp"
#include "format.hpp"
#include "../Utils/Http.hpp"
#include "I18N.hpp"

#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <curl/curl.h>
#include <string>

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

namespace Slic3r {
namespace GUI {

wxDEFINE_EVENT(EVT_OPEN_PRUSAAUTH, OpenPrusaAuthEvent);
wxDEFINE_EVENT(EVT_UA_LOGGEDOUT, UserAccountSuccessEvent);
wxDEFINE_EVENT(EVT_UA_ID_USER_SUCCESS, UserAccountSuccessEvent);
wxDEFINE_EVENT(EVT_UA_SUCCESS, UserAccountSuccessEvent);
wxDEFINE_EVENT(EVT_UA_PRUSACONNECT_PRINTERS_SUCCESS, UserAccountSuccessEvent);
wxDEFINE_EVENT(EVT_UA_AVATAR_SUCCESS, UserAccountSuccessEvent); 
wxDEFINE_EVENT(EVT_UA_FAIL, UserAccountFailEvent);
wxDEFINE_EVENT(EVT_UA_RESET, UserAccountFailEvent);

void UserActionPost::perform(/*UNUSED*/ wxEvtHandler* evt_handler, /*UNUSED*/ const std::string& access_token, UserActionSuccessFn success_callback, UserActionFailFn fail_callback, const std::string& input) const
{
    std::string url = m_url;
    auto http = Http::post(std::move(url));
    if (!input.empty())
        http.set_post_body(input);
    http.header("Content-type", "application/x-www-form-urlencoded");
    http.on_error([fail_callback](std::string body, std::string error, unsigned status) {
        if (fail_callback)
            fail_callback(body);
    });
    http.on_complete([success_callback](std::string body, unsigned status) {
        if (success_callback)
            success_callback(body);
    });
    http.perform_sync();
}

void UserActionGetWithEvent::perform(wxEvtHandler* evt_handler, const std::string& access_token, UserActionSuccessFn success_callback, UserActionFailFn fail_callback, const std::string& input) const
{
    std::string url = m_url + input;
    auto http = Http::get(std::move(url));
    if (!access_token.empty())
        http.header("Authorization", "Bearer " + access_token);
    http.on_error([evt_handler, fail_callback, action_name = &m_action_name, fail_evt_type = m_fail_evt_type](std::string body, std::string error, unsigned status) {
        if (fail_callback)
            fail_callback(body);
        std::string message = GUI::format("%1% action failed (%2%): %3%", action_name, std::to_string(status), body);
        if (fail_evt_type != wxEVT_NULL)
            wxQueueEvent(evt_handler, new UserAccountFailEvent(fail_evt_type, std::move(message)));
    });
    http.on_complete([evt_handler, success_callback, succ_evt_type = m_succ_evt_type](std::string body, unsigned status) {
        if (success_callback)
            success_callback(body);
        if (succ_evt_type != wxEVT_NULL)
            wxQueueEvent(evt_handler, new UserAccountSuccessEvent(succ_evt_type, body));
    });

    http.perform_sync();
}

void UserAccountSession::process_action_queue()
{
    if (!m_proccessing_enabled)
        return;
    if (m_priority_action_queue.empty() && m_action_queue.empty()) {
        // update printers periodically
        if (m_polling_enabled) {
            enqueue_action(UserAccountActionID::USER_ACCOUNT_ACTION_CONNECT_STATUS, nullptr, nullptr, {});
        } else {
            return;
        }
    }
    // priority queue works even when tokens are empty or broken
    while (!m_priority_action_queue.empty()) {
        m_actions[m_priority_action_queue.front().action_id]->perform(p_evt_handler, m_access_token, m_priority_action_queue.front().success_callback, m_priority_action_queue.front().fail_callback, m_priority_action_queue.front().input);
        if (!m_priority_action_queue.empty())
            m_priority_action_queue.pop();
    }
    // regular queue has to wait until priority fills tokens
    if (!this->is_initialized())
        return;
    while (!m_action_queue.empty()) {
        m_actions[m_action_queue.front().action_id]->perform(p_evt_handler, m_access_token, m_action_queue.front().success_callback, m_action_queue.front().fail_callback, m_action_queue.front().input);
        if (!m_action_queue.empty())
            m_action_queue.pop();
    }
}

void UserAccountSession::enqueue_action(UserAccountActionID id, UserActionSuccessFn success_callback, UserActionFailFn fail_callback, const std::string& input)
{
    m_proccessing_enabled = true;
    m_action_queue.push({ id, success_callback, fail_callback, input });
}


void UserAccountSession::init_with_code(const std::string& code, const std::string& code_verifier)
{
    // Data we have       
    const std::string REDIRECT_URI = "prusaslicer://login";
    std::string post_fields = "code=" + code +
        "&client_id=" + client_id() +
        "&grant_type=authorization_code" +
        "&redirect_uri=" + REDIRECT_URI +
        "&code_verifier="+ code_verifier;

    m_proccessing_enabled = true;
    // fail fn might be cancel_queue here
    m_priority_action_queue.push({ UserAccountActionID::USER_ACCOUNT_ACTION_CODE_FOR_TOKEN
        , std::bind(&UserAccountSession::token_success_callback, this, std::placeholders::_1)
        , std::bind(&UserAccountSession::code_exchange_fail_callback, this, std::placeholders::_1)
        , post_fields });
}

void UserAccountSession::token_success_callback(const std::string& body)
{
    // Data we need
    std::string access_token, refresh_token, shared_session_key;
    try {
        std::stringstream ss(body);
        pt::ptree ptree;
        pt::read_json(ss, ptree);

        const auto access_token_optional = ptree.get_optional<std::string>("access_token");
        const auto refresh_token_optional = ptree.get_optional<std::string>("refresh_token");
        const auto shared_session_key_optional = ptree.get_optional<std::string>("shared_session_key");

        if (access_token_optional)
            access_token = *access_token_optional;
        if (refresh_token_optional)
            refresh_token = *refresh_token_optional;
        if (shared_session_key_optional)
            shared_session_key = *shared_session_key_optional;
    }
    catch (const std::exception&) {
        std::string msg = "Could not parse server response after code exchange.";
        wxQueueEvent(p_evt_handler, new UserAccountFailEvent(EVT_UA_RESET, std::move(msg)));
        return;
    }

    if (access_token.empty() || refresh_token.empty() || shared_session_key.empty()) {
        // just debug msg, no need to translate
        std::string msg = GUI::format("Failed read tokens after POST.\nAccess token: %1%\nRefresh token: %2%\nShared session token: %3%\nbody: %4%", access_token, refresh_token, shared_session_key, body);
        m_access_token = std::string();
        m_refresh_token = std::string();
        m_shared_session_key = std::string();
        wxQueueEvent(p_evt_handler, new UserAccountFailEvent(EVT_UA_RESET, std::move(msg)));
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "access_token: " << access_token;
    BOOST_LOG_TRIVIAL(info) << "refresh_token: " << refresh_token;
    BOOST_LOG_TRIVIAL(info) << "shared_session_key: " << shared_session_key;

    m_access_token = access_token;
    m_refresh_token = refresh_token;
    m_shared_session_key = shared_session_key;
    enqueue_action(UserAccountActionID::USER_ACCOUNT_ACTION_USER_ID, nullptr, nullptr, {});
}

void UserAccountSession::code_exchange_fail_callback(const std::string& body)
{
    clear();
    cancel_queue();
    // Unlike refresh_fail_callback, no event was triggered so far, do it. (USER_ACCOUNT_ACTION_CODE_FOR_TOKEN does not send events)
    wxQueueEvent(p_evt_handler, new UserAccountFailEvent(EVT_UA_RESET, std::move(body)));
}

void UserAccountSession::enqueue_test_with_refresh()
{
    // on test fail - try refresh
    m_proccessing_enabled = true;
    m_priority_action_queue.push({ UserAccountActionID::USER_ACCOUNT_ACTION_TEST_ACCESS_TOKEN, nullptr, std::bind(&UserAccountSession::enqueue_refresh, this, std::placeholders::_1), {} });
}

void UserAccountSession::enqueue_refresh(const std::string& body)
{
    assert(!m_refresh_token.empty());
    std::string post_fields = "grant_type=refresh_token" 
        "&client_id=" + client_id() +
        "&refresh_token=" + m_refresh_token;

    m_priority_action_queue.push({ UserAccountActionID::USER_ACCOUNT_ACTION_REFRESH_TOKEN
        , std::bind(&UserAccountSession::token_success_callback, this, std::placeholders::_1)
        , std::bind(&UserAccountSession::refresh_fail_callback, this, std::placeholders::_1)
        , post_fields });
}

void UserAccountSession::refresh_fail_callback(const std::string& body)
{
    clear();
    cancel_queue();
    // No need to notify UI thread here
    // backtrace: load tokens -> TEST_TOKEN fail (access token bad) -> REFRESH_TOKEN fail (refresh token bad)
    // USER_ACCOUNT_ACTION_TEST_ACCESS_TOKEN triggers EVT_UA_FAIL, we need also RESET
    wxQueueEvent(p_evt_handler, new UserAccountFailEvent(EVT_UA_RESET, std::move(body)));

}

void UserAccountSession::cancel_queue()
{
    while (!m_priority_action_queue.empty()) {
        m_priority_action_queue.pop();
    }
    while (!m_action_queue.empty()) {
        m_action_queue.pop();
    }
}

}} // Slic3r::GUI