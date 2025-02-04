import {
  AppBar,
  AutoCompleteSearch,
  Badge,
  CardContainer,
  CardHeader,
  CardRow,
  Checkbox,
  Chip,
  DataGroup,
  DataPoint,
  DatePicker,
  DateTimePicker,
  Divider,
  Drawer,
  DrawerTab,
  Footer,
  Icon,
  IconButton,
  InfoDisplay,
  Label,
  Modal,
  MuiButton,
  Select,
  MuiTimePicker,
  PageLayout,
  PageLoadingIndicator,
  PureSearch,
  FlexSelect,
  Slider,
  Switch,
  Tab,
  Table,
  TextField,
  TimePicker,
  lightTheme,
  darkTheme,
  MaintModeSlider,
  NumericInput,
  Progress,
} from '@flexgen/storybook';
import AlarmFaultContainer from './components/AlarmFaultContainer';
import ConfirmCancelButton from './components/ConfirmCancelButton';
import MaintActionControl from './components/MaintActionControl';
import MaintActionProgress from './components/MaintActionProgress';
import TrueFalseButtonSet from './components/TrueFalseButtonSet';

// FIXME: storybook should probably have consistent types
export interface IComponentHash {
  [key: string]: any;
}

const storybookComponents: IComponentHash = {
  AlarmFaultContainer,
  AppBar,
  AutoCompleteSearch,
  Badge,
  CardContainer,
  CardHeader,
  CardRow,
  Checkbox,
  Chip,
  DataGroup,
  DataPoint,
  DatePicker,
  DateTimePicker,
  Divider,
  Drawer,
  DrawerTab,
  Footer,
  Icon,
  IconButton,
  InfoDisplay,
  Label,
  Modal,
  MuiButton,
  Select,
  MuiTimePicker,
  PageLayout,
  PageLoadingIndicator,
  PureSearch,
  FlexSelect,
  Slider,
  Switch,
  Tab,
  Table,
  TextField,
  TimePicker,
  lightTheme,
  darkTheme,
  MaintModeSlider,
  NumericInput,
  Progress,
  ConfirmCancelButton,
  MaintActionControl,
  MaintActionProgress,
  TrueFalseButtonSet,
  TrueFalseMaintModeButtonSet: TrueFalseButtonSet,
};

export default storybookComponents;

const controlsSets = {
  onChange: [
    'Checkbox',
    'Slider',
    'PureSearch',
    'AutoCompleteSearch',
    'DatePicker',
    'DateTimePicker',
    'TimePicker',
    'MuiTimePicker',
  ],
  onClick: ['IconButton', 'MuiButton', 'ConfirmCancelButton', 'MaintActionControl'],
  withConfirm: ['TextField', 'Switch', 'MaintModeSlider', 'Select', 'NumericInput'],
  onClickHandlers: ['TrueFalseButtonSet', 'TrueFalseMaintModeButtonSet'],
};

export { controlsSets };
