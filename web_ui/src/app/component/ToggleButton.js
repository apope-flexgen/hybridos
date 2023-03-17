import React, { useState } from 'react';

import { Button } from '@mui/material';

// TODO: integrate and finalize

/*
    Useful Props:
    -color
    -disabled
    -onClick
    -style
    -disableElevation

    These are default material ui buttons but not immediately obvious because of spread operator
*/
function ToggleButton(props) {
    const [active, setActive] = useState(false);

    const handleClick = () => {
        setActive(!active);
        if (props.handleClick) props.handleClick();
    }

    return (
        <Button
            variant={active ? 'contained' : 'outlined'}
            onClick={handleClick}
            {...props}
        >
            {props.startIcon}
            {props.text}
            {props.endIcon}
        </Button>
    )
}

export default ToggleButton;