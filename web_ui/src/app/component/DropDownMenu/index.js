/* eslint-disable no-underscore-dangle */
/* eslint-disable react/prop-types */
import React from 'react';
import { withStyles } from 'tss-react/mui';
import MenuItem from '@mui/material/MenuItem';
import InputLabel from '@mui/material/InputLabel';
import FormHelperText from '@mui/material/FormHelperText';
import FormControl from '@mui/material/FormControl';
import Select from '@mui/material/Select';

import { STYLES_FEATURES } from '../../styles';

/**
 * Component for rendering a dropdown menu
 */
class DropDownMenu extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
    }

    componentDidMount() {
        this._isMounted = true;
    }

    componentWillUnmount() {
        this._isMounted = false;
    }

    render() {
        return (
            <FormControl style={{ minWidth: 400 }}>
                <InputLabel id="input-label">{this.props.label}</InputLabel>
                <Select
                    labelId="input-label"
                    label={this.props.label}
                    value={this.props.value}
                    onChange={this.props.getStateFromChild}
                >
                    {this.props.menuItems.map((menuItem, i) => (
                        <MenuItem key={i} value={menuItem}>{menuItem}</MenuItem>
                    ))}
                </Select>
                <FormHelperText>{this.props.helperText}</FormHelperText>
            </FormControl>
        );
    }
}
export default withStyles(DropDownMenu, STYLES_FEATURES);
