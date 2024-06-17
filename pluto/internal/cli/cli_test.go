package cli

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

// TestParse tests the Parse function.
func TestParse(t *testing.T) {
	tests := []struct {
		name    string
		args    []string
		want    *Config
		wantErr bool
	}{
		{
			name:    "hello flag",
			args:    []string{"-hello"},
			want:    &Config{Hello: true},
			wantErr: false,
		},
		{
			name:    "nofims flag",
			args:    []string{"-nofims"},
			want:    &Config{NoFims: true},
			wantErr: false,
		},
		{
			name:    "no flags",
			args:    []string{},
			want:    &Config{},
			wantErr: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := Parse(tt.args)
			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.Equal(t, tt.want, got)
			}
		})
	}
}
