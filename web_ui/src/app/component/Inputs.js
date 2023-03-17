import React, { useState } from 'react';

import {
    FormControlLabel,
    Switch,
    Button,
    TextField,
    IconButton,
    Fab,
    CircularProgress,
    Typography
} from '@mui/material';

import {
    Visibility,
    VisibilityOff,
    Check
} from '@mui/icons-material';

function SwitchInput(props) {
    return (
        <FormControlLabel
            control={
                <Switch
                    checked={props.value}
                    onChange={(event) => props.handleChange(event)}
                    color={props.URI === '/maint_mode' ? "warning" : "primary"}
                    disabled={props.disabled}
                />
            }
            label={props.label}
            labelPlacement={props.labelPlacement}
        />
    )
}

function ButtonInput(props) {
    return (
        <Button
            size="small"
            color={props.on ? "warning" : "primary"}
            variant={props.value ? 'outlined' : 'contained'}
            onClick={() => props.handleChange()}
            disabled={props.disabled}
        >
            {props.label}
        </Button>
    )
}

function TextInput(props) {
    const [showPassword, setShowPassword] = useState(false);

    return (
        <TextField
            size="small"
            label={props.label}
            variant="outlined"
            onChange={(event) => props.handleChange(event)}
            value={props.value}
            disabled={props.disabled}
            InputLabelProps={{ shrink: true }}
            type={props.password && (showPassword ? 'text' : 'password')}
            helperText={props.helperText}
            InputProps={props.password && {
                endAdornment: <IconButton onClick={() => setShowPassword(!showPassword)} edge="end" size="large">
                    {props.password && (showPassword ? <VisibilityOff /> : <Visibility />)}
                </IconButton>
            }}
        />
    );
}

function NumberInput(props) {
    const [showPassword, setShowPassword] = useState(false);

    return (
        <TextField
            size="small"
            label={props.label}
            variant="outlined"
            onChange={(event) => props.handleChange(event)}
            value={props.value}
            disabled={props.disabled}
            InputLabelProps={{ shrink: true }}
            helperText={props.helperText}
            type="number"   // should be the correct type
            // type={props.password && (showPassword ? 'text' : 'password')}
            InputProps={props.password && {
                endAdornment: <IconButton
                    onClick={() => setShowPassword(!showPassword)}
                    edge="end"
                >
                    {props.password && (showPassword ? <VisibilityOff /> : <Visibility />)}
                </IconButton>
            }}
        />
    );
}

// Used for loading status with a button
function ProgressButtonInput(props) {
    return (
        <div style={{ position: 'relative' }}>
            <Fab
                // color={props.success ? 'green' : 'primary'}
                color="primary"
                onClick={() => props.handleChange()}
                size="medium"
                disabled={props.loading || props.success}
            >
                {props.success ? <Check /> : props.icon}
            </Fab>
            {props.loading &&
                <CircularProgress
                    size={56}
                    style={{
                        position: 'absolute',
                        top: -4,
                        left: -4,
                        zIndex: 1,
                    }}
                />
            }
        </div>
    )
}

export {
    SwitchInput,
    ButtonInput,
    TextInput,
    NumberInput,
    ProgressButtonInput,
};