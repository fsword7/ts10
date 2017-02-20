// commands.c - the console routines for commands and prompt
//
// Written by
//  Timothy Stark <sword7@speakeasy.org>
//
// This file is part of the TS-10 Emulator.
// See README for copyright notice.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "emu/defs.h"
#include "emu/socket.h"

extern MAP_DEVICE *ts10_Root;   // Root System (TS10 Emulator System)
extern MAP_DEVICE *ts10_System; // Selected System
extern MAP_DEVICE *ts10_Use;    // Selected Device

COMMAND ts10_Commands[];
COMMAND ts10_SetCommands[];
COMMAND ts10_ShowCommands[];

// If set, allow repetitive command to execute
// by just press CR key without any input.
int  ts10_LastCmdFlag = 0;
char ts10_LastCmd[TS10_CMDLEN];

int CmdHelp(void *, int, char **);

char *emu_ErrorMessages[] =
{
	"Non-existant Memory",
	"Memory Error",
	"Open Error",
	"I/O Error",
	"Unit is already present",
	"Unit is not present",
	"Unit is disabled",
	"Unit is not attachable",
	"Unit is already attached",
	"Invalid Argument",
	"Unknown Command",
	"Not Found",
	"Conflict",
	"Not Supported",
	"Not Bootable"
};

int CmdList(void *dev, int argc, char **argv)
{
	sock_ShowList();
	return EMU_OK;
}

int CmdLog(void *dev, int argc, char **argv)
{
	if (argc != 2)
		printf("Usage: log <filename|off>\n");
	else {
		RemoveSpaces(argv[1]);
		if (!strcasecmp(argv[1], "off")) {
			close(emu_logFile);
			emu_logFile = -1;
			return EMU_OK;
		}

		if (emu_logFile >= 0) {
			printf("Log file already was opened.\n");
			return EMU_OK;
		}

		if ((emu_logFile = open(argv[1], O_CREAT|O_WRONLY|O_APPEND, 0700)) < 0) {
			printf("log: %s: %s\n", argv[1], strerror(errno));
			return EMU_OK;
		}
	}
	return EMU_OK;
}

int CmdRun(void *dev, int argc, char **argv)
{
	if (argc != 1)
		printf("Usage: run\n");
	else {
		printf("Running now...\n");
		emu_State = EMU_RUN;
	}
	return EMU_OK;
}

int CmdSet(void *dev, int argc, char **argv)
{
	COMMAND *cmd;
	int     idx;

	if (argc < 2 ) {
		printf("Usage: %s <subcommand> ...\n", argv[0]);
		return EMU_OK;
	}

	// Search table for desired device command.
	if (ts10_Use && (cmd = ts10_Use->devInfo->SetCommands)) {
		for (idx = 0; cmd[idx].Name; idx++) {
			if (!strncasecmp(cmd[idx].Name, argv[1], strlen(argv[1])))
				return cmd[idx].Execute(ts10_Use->Device, argc, argv);
		}
	}

	// Search table for desired system command.
	if (ts10_System && (cmd = ts10_System->devInfo->SetCommands)) {
		for (idx = 0; cmd[idx].Name; idx++) {
			if (!strncasecmp(cmd[idx].Name, argv[1], strlen(argv[1])))
				return cmd[idx].Execute(ts10_System->Device, argc, argv);
		}
	}

	// Search table for desired general command.
	cmd = ts10_Root->devInfo->SetCommands;
	for (idx = 0; cmd[idx].Name; idx++) {
		if (!strncasecmp(cmd[idx].Name, argv[1], strlen(argv[1])))
			return cmd[idx].Execute(ts10_Root->Device, argc, argv);
	}

	return EMU_UNKNOWN;
}

int CmdShow(void *dev, int argc, char **argv)
{
	COMMAND *cmd;
	int     idx;

	if (argc < 2 ) {
		printf("Usage: %s <subcommand> ...\n", argv[0]);
		return EMU_OK;
	}

	// Search table for desired device command.
	if (ts10_Use && (cmd = ts10_Use->devInfo->ShowCommands)) {
		for (idx = 0; cmd[idx].Name; idx++) {
			if (!strncasecmp(cmd[idx].Name, argv[1], strlen(argv[1])))
				return cmd[idx].Execute(ts10_Use->Device, argc, argv);
		}
	}

	// Search table for desired system command.
	if (ts10_System && (cmd = ts10_System->devInfo->ShowCommands)) {
		for (idx = 0; cmd[idx].Name; idx++) {
			if (!strncasecmp(cmd[idx].Name, argv[1], strlen(argv[1])))
				return cmd[idx].Execute(ts10_System->Device, argc, argv);
		}
	}

	// Search table for desired general command.
	cmd = ts10_Root->devInfo->ShowCommands;
	for (idx = 0; cmd[idx].Name; idx++) {
		if (!strncasecmp(cmd[idx].Name, argv[1], strlen(argv[1])))
			return cmd[idx].Execute(ts10_Root->Device, argc, argv);
	}

	return EMU_UNKNOWN;
}

int CmdStart(void *dev, int argc, char **argv)
{
	if (argc != 2)
		printf("Usage: start <address>\n");
	else {
//		pdp10_Start(argv[1]);
//		emu_State = EMU_RUN;
	}
	return EMU_OK;
}

int CmdQuit(void *dev, int argc, char **argv)
{
	ts10_Exit("Quit Command");
}

COMMAND ts10_Commands[] =
{
	{ "attach",    "<device> ...", CmdAttach  },
#ifdef DEBUG
	{ "break",     "[switch] <address> [action]", CmdBreak },
#endif /* DEBUG */
	{ "boot",      "<device> ...", CmdBoot    },
	{ "configure", "<device> ...", CmdConfigure },
	{ "continue",  "",             CmdRun     },
	{ "create",    "<device> <type> ...", CmdCreate },
#ifdef DEBUG
	{ "debug",     "",             CmdDebug   },
	{ "debug2",    "<device> ...", CmdDebug2  },
#endif /* DEBUG */
//	{ "delete",    "",             CmdDelete  },
	{ "detach",    "",             CmdDetach  },
	{ "devices",   "",             CmdListDevice },
	{ "exit",      "",             CmdQuit    },
//	{ "format",    "",             CmdFormat  },
	{ "help",      "",             CmdHelp    },
#ifdef DEBUG
	{ "history",   "<on|off>",     CmdHistory },
#endif /* DEBUG */
	{ "info",      "",             CmdInfo    },
//	{ "init",      "",             CmdInit    },
	{ "list",      "",             CmdList    },
	{ "log",       "",             CmdLog     },
#ifdef DEBUG
	{ "nobreak",   "[switch] <address>", CmdNoBreak },
#endif /* DEBUG */
	{ "quit",      "",             CmdQuit    },
	{ "run",       "",             CmdRun     },
	{ "select",    "[system|none]",    CmdSelect },
	{ "set",       "<subcommand> ...", CmdSet },
	{ "show",      "<subcommand> ...", CmdShow },
	{ "set2",      "<device> <subcommand> ...", CmdSet2 },
	{ "show2",     "<device> [subcommand] ...", CmdShow2 },
	{ "start",     "",             CmdStart   },
#ifdef DEBUG
	{ "trace",     "<on|off>",     CmdTrace   },
#endif /* DEBUG */
	{ "use",       "[device|none]", CmdUse    },
	{ NULL }
};

COMMAND ts10_SetCommands[] =
{
	{ NULL }
};

COMMAND ts10_ShowCommands[] =
{
	{ "device",  "", CmdShowDevice },
	{ NULL }
};

void ListCommands(MAP_DEVICE *mapInfo)
{
	DEVICE  *devInfo  = mapInfo->devInfo;
	COMMAND *Cmds     = devInfo->Commands;
	COMMAND *SetCmds  = devInfo->SetCommands;
	COMMAND *ShowCmds = devInfo->ShowCommands;
	int     idx;

	// Display information now.
	printf("%s: - %s %s\n",
		mapInfo->devName, devInfo->emuName, devInfo->emuVersion);

	if (Cmds != NULL) {
		printf("\n%s Commands:\n\n", mapInfo->devName);
		printf("  Command          Usage\n");
		printf("  -------          -----\n");
		for (idx = 0; Cmds[idx].Name; idx++)
			printf("  %-16s %s\n", Cmds[idx].Name, Cmds[idx].Usage);
	}

	if (SetCmds != NULL) {
		printf("\n%s Set Commands:\n\n", mapInfo->devName);
		printf("  Command          Usage\n");
		printf("  -------          -----\n");
		for (idx = 0; SetCmds[idx].Name; idx++)
			printf("  %-16s %s\n", SetCmds[idx].Name, SetCmds[idx].Usage);
	}

	if (ShowCmds != NULL) {
		printf("\n%s Show Commands:\n\n", mapInfo->devName);
		printf("  Command          Usage\n");
		printf("  -------          -----\n");
		for (idx = 0; ShowCmds[idx].Name; idx++)
			printf("  %-16s %s\n", ShowCmds[idx].Name, ShowCmds[idx].Usage);
	}
}

void ListAllCommands(void)
{
	ListCommands(ts10_Root);
	if (ts10_System)
		ListCommands(ts10_System);
	if (ts10_Use)
		ListCommands(ts10_Use);
}

int ExecuteCommand(char *line)
{
	COMMAND *cmd;
	int     argc;
	char    *argv[TS10_MAXARGS];
	int     idx;

	if (ts10_LastCmdFlag && *line == '\0') {
		// Only one argument needed to allow the repetitive
		// command with advancing options/switches that
		// already was parsed by last command.
		ts10_LastCmdFlag = 0;
		argv[0] = ts10_LastCmd;
		argc    = 1;
	} else {
		// Break a line into a list of arguments.
		for (argc = 0; (*line && argc < TS10_MAXARGS); argc++)
			argv[argc] = SplitWord(&line);
	}

	if (argc > 0) {
		// Search table for desired device command.
		if (ts10_Use && (cmd = ts10_Use->devInfo->Commands)) {
			for (idx = 0; cmd[idx].Name; idx++) {
				if (!strncasecmp(cmd[idx].Name, argv[0], strlen(argv[0])))
					return cmd[idx].Execute(ts10_Use->Device, argc, argv);
			}
		}

		// Search table for desired system command.
		if (ts10_System && (cmd = ts10_System->devInfo->Commands)) {
			for (idx = 0; cmd[idx].Name; idx++) {
				if (!strncasecmp(cmd[idx].Name, argv[0], strlen(argv[0])))
					return cmd[idx].Execute(ts10_System->Device, argc, argv);
			}
		}

		// Search table for desired general command.
		cmd = ts10_Root->devInfo->Commands;
		for (idx = 0; cmd[idx].Name; idx++) {
			if (!strncasecmp(cmd[idx].Name, argv[0], strlen(argv[0])))
				return cmd[idx].Execute(ts10_Root->Device, argc, argv);
		}

		return EMU_UNKNOWN;
	}

	return EMU_OK;
}

void ExecutePrompt(char *buf)
{
	int  st;

	if (st = ExecuteCommand(buf)) {
		if (st >= EMU_BASE) {
			if (st == EMU_OPENERR)
				printf("%s: %s\n", emu_ErrorMessages[st - EMU_BASE],
					strerror(errno));
			else
				printf("%s\n", emu_ErrorMessages[st - EMU_BASE]);
		}
		return;
	}
}

void ExecuteFile(char *filename)
{
	FILE *config;
	char line[256];
	int  lineno = 0;
	int  len;
	int  idx;
	int  st;

	if ((config = fopen(filename, "r")) == NULL) {
		perror(filename);
		return;
	}

	while (!feof(config)) {
		if (fgets(line, 256, config) != NULL) {
			lineno++;

			// Remove '\n' from end of each line.
			len = strlen(line)-1;
			line[len] = '\0';

			// Ignore all comments after ';' character
			for (idx = 0; idx < len; idx++)
				if (line[idx] == ';')
					break;
			line[idx] = '\0';

			RemoveSpaces(line);

			// Now execute commands
			if (line[0]) {
				if (st = ExecuteCommand(line)) {
					if (st >= EMU_BASE) {
						if (st == EMU_OPENERR)
							printf("%s: %s\n", emu_ErrorMessages[st - EMU_BASE],
								strerror(errno));
						else
							printf("%s\n", emu_ErrorMessages[st - EMU_BASE]);
					}
					printf("Error occurs at line %d\n", lineno);
				}
			}
		}
	}

	fclose(config);
}
