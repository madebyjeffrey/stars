package main

import (
	"fmt"
	"github.com/spf13/cobra"
	"stars-save/savefile"
)

// newSaveUpdateCmd returns the command that assigns Maid AI in a turn or host file.
func newSaveUpdateCmd() *cobra.Command {
	var ai string
	var player int
	cmd := &cobra.Command{
		Use:   "update <file>",
		Short: "Update a player turn or host file",
		Args:  cobra.ExactArgs(1),
		RunE: func(cmd *cobra.Command, args []string) error {
			if ai != "maid" {
				return fmt.Errorf("--ai must be maid")
			}
			if cmd.Flags().Changed("player") {
				if player < 1 || player > 16 {
					return fmt.Errorf("--player must be between 1 and 16")
				}
				return savefile.UpdatePlayerAI(args[0], player-1)
			}
			return savefile.UpdatePlayerAI(args[0], -1)
		},
	}
	cmd.Flags().StringVar(&ai, "ai", "", "AI to assign (maid)")
	cmd.Flags().IntVar(&player, "player", 0, "player number (1-16; required for host files)")
	_ = cmd.MarkFlagRequired("ai")
	return cmd
}
