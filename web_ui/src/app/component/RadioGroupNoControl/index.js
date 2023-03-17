/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React from 'react';
import { withStyles } from 'tss-react/mui';
import Radio from '@mui/material/Radio';
import RadioGroup from '@mui/material/RadioGroup';
import FormControlLabel from '@mui/material/FormControlLabel';
import FormControl from '@mui/material/FormControl';
import FormLabel from '@mui/material/FormLabel';
import { STYLES_FEATURES } from '../../styles';

// this component differs from RadioGroupControl in that it does not
// require confirmation when selecting a new radio button, and it does
// not update anything via updatePropertyValue from AppConfig
// It gets and sends radio button values from/to a parent component.
// Currently, Layout is the only parent that makes use of this component.
// See Layout for getStateFromChild details.
/**
 * Radio button with no confirmation or logic
 */
class RadioGroupNoControl extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
    }

    render() {
        const { control } = this.props;
        if (control.name.toLowerCase().includes('factor')) {
            return (
                <FormControl>
                    <FormLabel>{control.name}</FormLabel>
                    <RadioGroup
                        style={{ marginTop: 10 }}
                        aria-label={control.id}
                        name={control.name.toLowerCase().replace(/ /g, '_')}
                        value={control.value || ''}
                        onClick={this.props.getStateFromChild2} >
                        {control.options.map((element) => (
                            <FormControlLabel
                                style={{ margin: -10 }}
                                key={element.name.toLowerCase()}
                                value={element.return_value}
                                control={<Radio color="default" />}
                                label={element.name}
                                data-cy={element.name.toLowerCase().replace(/ /g, '_')}
                            />
                        ))}
                    </RadioGroup>
                </FormControl>
            );
        }
        return (
            <FormControl style={{
                width: '12em', marginTop: 10, padding: 10, backgroundColor: '#dedede', marginBottom: 20,
            }}>
                <FormLabel>{control.name}</FormLabel>
                <RadioGroup
                    style={{ marginTop: 10 }}
                    aria-label={control.id}
                    name={control.name.toLowerCase().replace(/ /g, '_')}
                    value={control.value || ''}
                    onClick={this.props.getStateFromChild} >
                    {control.options.map((element) => (
                        <FormControlLabel
                            style={{ margin: -10 }}
                            key={element.name.toLowerCase()}
                            value={element.value}
                            control={<Radio color="default" />}
                            label={element.name}
                            data-cy={element.name.toLowerCase().replace(/ /g, '_')}
                        />
                    ))}
                </RadioGroup>
            </FormControl>
        );
    }
}
export default withStyles(RadioGroupNoControl, STYLES_FEATURES);
