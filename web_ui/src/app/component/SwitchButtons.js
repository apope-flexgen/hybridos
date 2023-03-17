import React, { useState } from 'react';
import { ButtonInput } from './Inputs';

function SwitchButtons(props) {
    return (

        <>
            <ButtonInput
                value={props.switchOn}
                handleChange={() => {
                    props.handleSwitchButtons('on');
                }}
                label = {`${props.name} ON`}
                disabled={props.disabled}
                on = {true}
            />
            <ButtonInput
                value={props.switchOff}
                handleChange={() => {
                    props.handleSwitchButtons('off')
                }}
                label={`${props.name} OFF`}
                disabled={props.disabled}
                on = {false}
            />
        </>
        
    )
}

export default SwitchButtons;