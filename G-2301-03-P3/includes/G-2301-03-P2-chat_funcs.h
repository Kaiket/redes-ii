#ifndef __CHAT_FUNCS_H
#define __CHAT_FUNCS_H

#define DFLT_PORT 6667
#define BUFFER 512

#define IRC_MSG 0
#define IRC_CMD 1

#define RPL_WELCOME_STR "001"
#define RPL_YOURHOST_STR "002"
#define RPL_CREATED_STR "003"
#define RPL_MYINFO_STR "004"

#define RPL_TRACELINK_STR "200"
#define RPL_TRACECONNECTING_STR "201"
#define RPL_TRACEHANDSHAKE_STR "202"
#define RPL_TRACEUNKNOWN_STR "203"
#define RPL_TRACEOPERATOR_STR "204"
#define RPL_TRACEUSER_STR "205"
#define RPL_TRACESERVER_STR "206"
#define RPL_TRACENEWTYPE_STR "208"
#define RPL_TRACELOG_STR "261"
#define RPL_STATSLINKINFO_STR "211"
#define RPL_STATSCOMMANDS_STR "212"
#define RPL_STATSCLINE_STR "213"
#define RPL_STATSNLINE_STR "214"
#define RPL_STATSILINE_STR "215"
#define RPL_STATSKLINE_STR "216"
#define RPL_STATSYLINE_STR "218"
#define RPL_ENDOFSTATS_STR "219"
#define RPL_STATSLLINE_STR "241"
#define RPL_STATSUPTIME_STR "242"
#define RPL_STATSOLINE_STR "243"
#define RPL_STATSHLINE_STR "244"
#define RPL_UMODEIS_STR "221"
#define RPL_LUSERCLIENT_STR "251"
#define RPL_LUSEROP_STR "252"
#define RPL_LUSERUNKNOWN_STR "253"
#define RPL_LUSERCHANNELS_STR "254"
#define RPL_LUSERME_STR "255"
#define RPL_ADMINME_STR "256"
#define RPL_ADMINLOC1_STR "257"
#define RPL_ADMINLOC2_STR "258"
#define RPL_ADMINEMAIL_STR "259"

#define RPL_NONE_STR "300"
#define RPL_USERHOST_STR "302"
#define RPL_ISON_STR "303"
#define RPL_AWAY_STR "301"
#define RPL_UNAWAY_STR "305"
#define RPL_NOWAWAY_STR "306"
#define RPL_WHOISUSER_STR "311"
#define RPL_WHOISSERVER_STR "312"
#define RPL_WHOISOPERATOR_STR "313"
#define RPL_WHOISIDLE_STR "317"
#define RPL_ENDOFWHOIS_STR "318"
#define RPL_WHOISCHANNELS_STR "319"
#define RPL_WHOWASUSER_STR "314"
#define RPL_ENDOFWHOWAS_STR "369"
#define RPL_LISTSTART_STR "321"
#define RPL_LIST_STR "322"
#define RPL_LISTEND_STR "323"
#define RPL_CHANNELMODEIS_STR "324"
#define RPL_NOTOPIC_STR "331"
#define RPL_TOPIC_STR "332"
#define RPL_INVITING_STR "341"
#define RPL_SUMMONING_STR "342"
#define RPL_VERSION_STR "351"
#define RPL_WHOREPLY_STR "352"
#define RPL_ENDOFWHO_STR "315"
#define RPL_NAMREPLY_STR "353"
#define RPL_ENDOFNAMES_STR "366"
#define RPL_LINKS_STR "364"
#define RPL_ENDOFLINKS_STR "365"
#define RPL_BANLIST_STR "367"
#define RPL_ENDOFBANLIST_STR "368"
#define RPL_INFO_STR "371"
#define RPL_ENDOFINFO_STR "374"
#define RPL_MOTDSTART_STR "375"
#define RPL_MOTD_STR "372"
#define RPL_ENDOFMOTD_STR "376"
#define RPL_YOUREOPER_STR "381"
#define RPL_REHASHING_STR "382"
#define RPL_TIME_STR "391"
#define RPL_USERSSTART_STR "392"
#define RPL_USERS_STR "393"
#define RPL_ENDOFUSERS_STR "394"
#define RPL_NOUSERS_STR "395"

#define ERR_NOSUCHNICK_STR "401"
#define ERR_NOSUCHSERVER_STR "402"
#define ERR_NOSUCHCHANNEL_STR "403"
#define ERR_CANNOTSENDTOCHAN_STR "404"
#define ERR_TOOMANYCHANNELS_STR "405"
#define ERR_WASNOSUCHNICK_STR "406"
#define ERR_TOOMANYTARGETS_STR "407"
#define ERR_NOORIGIN_STR "409"
#define ERR_NORECIPIENT_STR "411"
#define ERR_NOTEXTTOSEND_STR "412"
#define ERR_NOTOPLEVEL_STR "413"
#define ERR_WILDTOPLEVEL_STR "414"
#define ERR_UNKNOWNCOMMAND_STR "421"
#define ERR_NOMOTD_STR "422"
#define ERR_NOADMININFO_STR "423"
#define ERR_FILEERROR_STR "424"
#define ERR_NONICKNAMEGIVEN_STR "431"
#define ERR_ERRONEUSNICKNAME_STR "432"
#define ERR_NICKNAMEINUSE_STR "433"
#define ERR_NICKCOLLISION_STR "436"
#define ERR_USERNOTINCHANNEL_STR "441"
#define ERR_NOTONCHANNEL_STR "442"
#define ERR_USERONCHANNEL_STR "443"
#define ERR_NOLOGIN_STR "444"
#define ERR_SUMMONDISABLED_STR "445"
#define ERR_USERSDISABLED_STR "446"
#define ERR_NOTREGISTERED_STR "451"
#define ERR_NEEDMOREPARAMS_STR "461"
#define ERR_ALREADYREGISTRED_STR "462"
#define ERR_NOPERMFORHOST_STR "463"
#define ERR_PASSWDMISMATCH_STR "464"
#define ERR_YOUREBANNEDCREEP_STR "465"
#define ERR_KEYSET_STR "467"
#define ERR_CHANNELISFULL_STR "471"
#define ERR_UNKNOWNMODE_STR "472"
#define ERR_INVITEONLYCHAN_STR "473"
#define ERR_BANNEDFROMCHAN_STR "474"
#define ERR_BADCHANNELKEY_STR "475"
#define ERR_ILLEGALCHNAME_STR "479"
#define ERR_NOPRIVILEGES_STR "481"
#define ERR_CHANOPRIVSNEEDED_STR "482"
#define ERR_CANTKILLSERVER_STR "483"
#define ERR_RESTRICTED_STR "484"
#define ERR_NOOPERHOST_STR "491"
#define ERR_UMODEUNKNOWNFLAG_STR "501"
#define ERR_USERSDONTMATCH_STR "502"

#define NOTICE_CMD_STR "NOTICE"
#define JOIN_CMD_STR "JOIN"
#define NICK_CMD_STR "NICK"
#define PART_CMD_STR "PART"
#define QUIT_CMD_STR "QUIT"
#define PRIVMSG_CMD_STR "PRIVMSG"
#define INVITE_CMD_STR "INVITE"
#define PING_CMD_STR "PING"
#define MODE_CMD_STR "MODE"
#define ERROR_CMD_STR "ERROR"
#define EXIT_CMD_STR "EXIT"
#define MSG_CMD_STR "MSG"
#define QUERY_CMD_STR "QUERY"
#define ME_CMD_STR "ME"
#define SERVER_CMD_STR "SERVER"
#define KICK_CMD_STR "KICK"
#define BAN_CMD_STR "BAN"

#define PCALL_CMD_STR "PCALL"
#define PACCEPT_CMD_STR "PACCEPT"
#define PCLOSE_CMD_STR "PCLOSE"

#define FSEND_CMD_STR "FSEND"
#define FACCEPT_CMD_STR "FACCEPT"
#define FCANCEL_CMD_STR "FCANCEL"

#endif