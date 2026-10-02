package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"stars-save/savefile"
)

// TestSaveCompareCommand exercises encryption normalization, coordinate differences, and invalid files through Cobra.
func TestSaveCompareCommand(t *testing.T) {
	dir := t.TempDir()
	left, right := filepath.Join(dir, "left.xy"), filepath.Join(dir, "right.xy")
	a := comparisonXY(t, savefile.DefaultTables(), 100, 37)
	b := comparisonXY(t, savefile.DefaultTables(), 999, 725)
	for _, tc := range []struct {
		name      string
		data      []byte
		wantError bool
		want      string
	}{
		{"different identities match", b, false, "MATCH (decrypted"},
		{"changed coordinates differ", append([]byte(nil), b...), true, "star 0 x"},
		{"malformed save fails", []byte("invalid"), true, ""},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if tc.name == "changed coordinates differ" {
				tc.data[84] ^= 1
			}
			for path, data := range map[string][]byte{left: a, right: tc.data} {
				if err := os.WriteFile(path, data, 0o600); err != nil {
					t.Fatal(err)
				}
			}
			cmd := newRootCmd()
			var out bytes.Buffer
			cmd.SetOut(&out)
			cmd.SetErr(&out)
			cmd.SetArgs([]string{"save", "compare", left, right})
			err := cmd.Execute()
			if (err != nil) != tc.wantError {
				t.Fatalf("error = %v; wantError = %v", err, tc.wantError)
			}
			if err != nil {
				out.WriteString(err.Error())
			}
			if !strings.Contains(out.String(), tc.want) {
				t.Fatalf("missing %q in %q", tc.want, out.String())
			}
		})
	}
}
