/*
 * Copyright (c) 2003 by FlashCode <flashcode@flashtux.org>
 *                       Bounga <bounga@altern.org>
 *                       Xahlexx <xahlexx@tuxisland.org>
 * See README for License detail.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* irc-commands.c: implementation of IRC commands, according to
                   RFC 1459,2810,2811,2812 */


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>

#include "../weechat.h"
#include "irc.h"
#include "../command.h"
#include "../config.h"
#include "../gui/gui.h"


t_irc_command irc_commands[] =
{ { "away", N_("toggle away status"),
    N_("[-all] [message]"),
    N_("-all: toggle away status on all connected servers\n"
    "message: message for away (if no message is given, away status is removed)"),
    0, MAX_ARGS, 1, NULL, irc_cmd_send_away, NULL },
  { "ctcp", N_("send a ctcp message"),
    N_("nickname type"),
    N_("nickname: user to send ctcp to\ntype: \"action\" or \"version\""),
    2, MAX_ARGS, 1, NULL, irc_cmd_send_ctcp, NULL },
  { "deop", N_("removes channel operator status from nickname(s)"),
    N_("nickname [nickname]"), "",
    1, 1, 1, irc_cmd_send_deop, NULL, NULL },
  { "devoice", N_("removes voice from nickname(s)"),
    N_("nickname [nickname]"), "",
    1, 1, 1, irc_cmd_send_devoice, NULL, NULL },
  { "error", N_("error received from IRC server"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_error },
  { "invite", N_("invite a nick on a channel"),
    N_("nickname channel"),
    N_("nickname: nick to invite\nchannel: channel to invite"),
    2, 2, 1, NULL, irc_cmd_send_invite, NULL },
  { "join", N_("join a channel"),
    N_("channel[,channel] [key[,key]]"),
    N_("channel: channel name to join\nkey: key to join the channel"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_join, irc_cmd_recv_join },
  { "kick", N_("forcibly remove a user from a channel"),
    N_("[channel] nickname [comment]"),
    N_("channel: channel where user is\nnickname: nickname to kick\ncomment: comment for kick"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_kick, irc_cmd_recv_kick },
  { "kill", N_("close client-server connection"),
    N_("nickname comment"),
    N_("nickname: nickname\ncomment: comment for kill"),
    2, MAX_ARGS, 1, NULL, irc_cmd_send_kill, NULL },
  { "list", N_("list channels and their topic"),
    N_("[channel[,channel] [server]]"),
    N_("channel: channel to list\nserver: server name"),
    0, MAX_ARGS, 1, NULL, irc_cmd_send_list, NULL },
  { "me", N_("send a ctcp action to the current channel"),
    N_("message"),
    N_("message: message to send"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_me, NULL },
  { "mode", N_("change channel or user mode"),
    N_("{ channel {[+|-]|o|p|s|i|t|n|b|v} [limit] [user] [ban mask] } | "
    "{ nickname {[+|-]|i|w|s|o}"),
    N_("channel modes:\n"
    "  channel: channel name to modify\n"
    "  o: give/take channel operator privileges\n"
    "  p: private channel flag\n"
    "  s: secret channel flag\n"
    "  i: invite-only channel flag\n"
    "  t: topic settable by channel operator only flag\n"
    "  n: no messages to channel from clients on the outside\n"
    "  m: moderated channel\n"
    "  l: set the user limit to channel\n"
    "  b: set a ban mask to keep users out\n"
    "  v: give/take the ability to speak on a moderated channel\n"
    "  k: set a channel key (password)\n"
    "user modes:\n"
    "  nickname: nickname to modify\n"
    "  i: mark a user as invisible\n"
    "  s: mark a user for receive server notices\n"
    "  w: user receives wallops\n"
    "  o: operator flag\n"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_mode, irc_cmd_recv_mode },
  { "msg", N_("send message to a nick or channel"),
    N_("receiver[,receiver] text"),
    N_("receiver: nick or channel (may be mask, '*' = current channel)"
    "\ntext: text to send"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_msg, NULL },
  { "names", N_("list nicknames on channels"),
    N_("[channel[,channel]]"), N_("channel: channel name"),
    0, MAX_ARGS, 1, NULL, irc_cmd_send_names, NULL },
  { "nick", N_("change current nickname"),
    N_("nickname"), N_("nickname: new nickname for current IRC server"),
    1, 1, 1, irc_cmd_send_nick, NULL, irc_cmd_recv_nick },
  { "notice", N_("send notice message to user"),
    N_("nickname text"), N_("nickname: user to send notice to\ntext: text to send"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_notice, irc_cmd_recv_notice },
  { "op", N_("gives channel operator status to nickname(s)"),
    N_("nickname [nickname]"), "",
    1, 1, 1, irc_cmd_send_op, NULL, NULL },
  { "oper", N_("get operator privileges"),
    N_("user password"),
    N_("user/password: used to get privileges on current IRC server"),
    2, 2, 1, irc_cmd_send_oper, NULL, NULL },
  { "part", N_("leave a channel"),
    N_("[channel[,channel]]"), N_("channel: channel name to leave"),
    0, MAX_ARGS, 1, NULL, irc_cmd_send_part, irc_cmd_recv_part },
  { "ping", N_("ping server"),
    N_("server1 [server2]"),
    N_("server1: server to ping\nserver2: forward ping to this server"),
    1, 2, 1, irc_cmd_send_ping, NULL, irc_cmd_recv_ping },
  { "pong", N_("answer to a ping message"),
    N_("daemon [daemon2]"), N_("daemon: daemon who has responded to Ping message\n"
    "daemon2: forward message to this daemon"),
    1, 2, 1, irc_cmd_send_pong, NULL, NULL },
  { "privmsg", N_("message received"),
    "", "",
    0, 0, 1, NULL, NULL, irc_cmd_recv_privmsg },
  { "quit", N_("close all connections & quit " WEECHAT_NAME),
    N_("[quit_message]"),
    N_("quit_message: quit message (displayed to other users)"),
    0, MAX_ARGS, 0, NULL, irc_cmd_send_quit, irc_cmd_recv_quit },
  { "quote", N_("send raw data to server without parsing"),
    N_("data"),
    N_("data: raw data to send"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_quote, NULL },
  { "rehash", N_("tell the server to reload its config file"),
    "", "",
    0, 0, 1, NULL, irc_cmd_send_rehash, NULL },
  { "restart", N_("tell the server to restart itself"),
    "", "",
    0, 0, 1, NULL, irc_cmd_send_restart, NULL },
  { "stats", N_("query statistics about server"),
    "[query [server]]",
    "query: c/h/i/k/l/m/o/y/u (see RFC1459)\nserver: server name",
    0, 2, 1, NULL, irc_cmd_send_stats, NULL },
  { "topic", N_("get/set channel topic"),
    N_("[channel] [topic]"), N_("channel: channel name\ntopic: new topic for channel "
    "(if topic is \"-delete\" then topic is deleted)"),
    0, MAX_ARGS, 1, NULL, irc_cmd_send_topic, irc_cmd_recv_topic },
  { "version", N_("gives the version info of nick or server (current or specified)"),
    N_("[server | nickname]"), N_("server: server name\nnickname: nickname"),
    0, 1, 1, NULL, irc_cmd_send_version, NULL },
  { "voice", N_("gives voice to nickname(s)"),
    N_("nickname [nickname]"), "",
    1, 1, 1, irc_cmd_send_voice, NULL, NULL },
  { "whois", N_("query information about user(s)"),
    N_("[server] nickname[,nickname]"), N_("server: server name\n"
    "nickname: nickname (may be a mask)"),
    1, MAX_ARGS, 1, NULL, irc_cmd_send_whois, NULL },
  { "001", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "002", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "003", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "004", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_004 },
  { "005", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "212", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "219", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "250", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "251", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "252", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "253", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "254", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "255", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "256", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "257", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "258", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "259", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "260", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "261", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "262", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "263", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "264", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "265", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "266", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "267", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "268", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "269", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "301", N_("away message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_301 },
  { "305", N_("unaway"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_reply },
  { "306", N_("now away"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_reply },
  { "311", N_("whois (user)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_311 },
  { "312", N_("whois (server)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_312 },
  { "313", N_("whois (operator)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_313 },
  { "317", N_("whois (idle)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_317 },
  { "318", N_("whois (end)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_318 },
  { "319", N_("whois (channels)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_319 },
  { "320", N_("whois (identified user)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_320 },
  { "321", N_("/list start"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_321 },  
  { "322", N_("channel (for /list)"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_322 },
  { "323", N_("/list end"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_323 },
  { "331", N_("no topic for channel"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_331 },
  { "332", N_("topic of channel"),
    N_("channel :topic"),
    N_("channel: name of channel\ntopic: topic of the channel"),
    2, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_332 },
  { "333", N_("infos about topic (nick & date changed)"),
    "", "",
    0, 0, 1, NULL, NULL, irc_cmd_recv_333 },
  { "351", N_("server version"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_351 },
  { "353", N_("list of nicks on channel"),
    N_("channel :[[@|+]nick ...]"),
    N_("channel: name of channel\nnick: nick on the channel"),
    2, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_353 },
  { "366", N_("end of /names list"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_366 },
  { "371", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "372", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "373", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "374", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "375", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "376", N_("a server message"), "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_server_msg },
  { "401", N_("no such nick/channel"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "402", N_("no such server"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "403", N_("no such channel"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "404", N_("cannot send to channel"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "405", N_("too many channels"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "406", N_("was no such nick"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "406", N_("was no such nick"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "407", N_("was no such nick"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "409", N_("no origin"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "411", N_("no recipient"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "412", N_("no text to send"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "413", N_("no toplevel"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "414", N_("wilcard in toplevel domain"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "421", N_("unknown command"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "422", N_("MOTD is missing"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "423", N_("no administrative info"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "424", N_("file error"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "431", N_("no nickname given"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "432", N_("erroneus nickname"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "433", N_("nickname already in use"),
    "", "", 0, 0, 1, NULL, NULL, irc_cmd_recv_433 },
  { "436", N_("nickname collision"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "441", N_("user not in channel"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "442", N_("not on channel"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "443", N_("user already on channel"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "444", N_("user not logged in"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "445", N_("summon has been disabled"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "446", N_("users has been disabled"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "451", N_("you are not registered"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "461", N_("not enough parameters"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "462", N_("you may not register"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "463", N_("your host isn't among the privileged"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "464", N_("password incorrect"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "465", N_("you are banned from this server"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "467", N_("channel key already set"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "471", N_("channel is already full"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "472", N_("unknown mode char to me"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "473", N_("cannot join channel (invite only)"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "474", N_("cannot join channel (banned from channel)"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "475", N_("cannot join channel (bad channel key)"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "481", N_("you're not an IRC operator"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "482", N_("you're not channel operator"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "483", N_("you can't kill a server!"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "491", N_("no O-lines for your host"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "501", N_("unknown mode flag"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { "502", N_("can't change mode for other users"),
    "", "", 0, MAX_ARGS, 1, NULL, NULL, irc_cmd_recv_error },
  { NULL, NULL, NULL, NULL, 0, 0, 1, NULL, NULL, NULL }
};
