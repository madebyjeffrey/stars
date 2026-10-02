package main

import (
	"github.com/spf13/cobra"
	"os"
)

// newRootCmd builds the standalone regression save-tool command tree.
func newRootCmd() *cobra.Command {
	root := &cobra.Command{Use: "stars-save", Short: "Stars! regression save tools", SilenceUsage: true}
	save := &cobra.Command{Use: "save", Short: "Compare and update encrypted Stars! saves"}
	save.AddCommand(newSaveCompareCmd(), newSaveUpdateCmd())
	root.AddCommand(save)
	return root
}

// main runs the CLI and reports command failures with a nonzero exit status.
func main() {
	if err := newRootCmd().Execute(); err != nil {
		os.Exit(1)
	}
}
