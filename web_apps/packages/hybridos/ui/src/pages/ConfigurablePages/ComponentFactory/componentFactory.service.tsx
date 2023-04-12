/* eslint-disable */
import storybookComponents, { controlsSets } from './storybookHash'
import {
  SingleReactComponentMetadata,
  ControlHandlerObject,
  PossibleConfirmValues,
  StatusComponentStateInfo,
  ControlComponentStateInfo,
  MemoizedComponentObject,
} from './componentFactory.types'
import SocketConnectionManager from '../../../services/SocketConnectionManager'
import {
  ConfigurableComponentFunction,
  ConfigurablePageStateStructure,
} from '../configurablePages.types'

// FIXME: debounce button inputs (should this be an option in our button component?)
const generateControlHandlerObject = (
  uri: string,
  componentName: string,
  scalar: number | null
): ControlHandlerObject => {
  if (controlsSets.onClick.includes(componentName)) {
    return {
      onClick: () => {
        // FIXME: probably should not access SocketConnectionManager directly here
        SocketConnectionManager.send('fimsNoReply', {
          method: 'set',
          uri,
          replyto: 'web_ui',
          body: true,
          username: 'web_ui',
        })
      },
    }
  }

  if (controlsSets.withConfirm.includes(componentName)) {
    return {
      onCheck: (value: PossibleConfirmValues) => {
        let trueValue = value
        const maybeNumber = Number(value)
        if (!isNaN(maybeNumber) && scalar !== null) {
          trueValue = maybeNumber * scalar
        }

        SocketConnectionManager.send('fimsNoReply', {
          method: 'set',
          uri,
          replyto: 'web_ui',
          body: trueValue,
          username: 'web_ui',
        })
      },
      onX: (value: PossibleConfirmValues) => {},
    }
  }

  return {}
}

// FIXME: any
const getSpecificStateInfo = (
  assetState: ConfigurablePageStateStructure,
  displayGroupID: string,
  componentID: string,
  control: boolean
): any => {
  if (control) {
    return assetState[displayGroupID].control[componentID]
  }

  return assetState[displayGroupID].status[componentID]
}

const memoIsValid = (
  memoized: MemoizedComponentObject,
  stateInfo: StatusComponentStateInfo | ControlComponentStateInfo | undefined
): boolean => {
  const { prevState } = memoized
  if (stateInfo === undefined) {
    return true
  }

  if (JSON.stringify(prevState) === JSON.stringify(stateInfo)) {
    return true
  }

  return false
}

// TODO: this could probably be done better
const organizeProps = (
  props: any,
  component: string,
  stateInfo: StatusComponentStateInfo | ControlComponentStateInfo | undefined
): any => {
  if (stateInfo === undefined) {
    return props
  }

  const [value, enabled] =
    typeof stateInfo === 'object' ? [stateInfo.value, stateInfo.enabled] : [stateInfo, true]

  if (component === 'TextField') {
    props.placeholder = value
  } else if (component === 'Switch' || component === 'MaintModeSlider') {
    props.value = value
  } else {
    if (typeof value === 'number') {
      props.value = value.toFixed(2)
    } else {
      props.value = String(value)
    }
  }
  props.disabled = !enabled

  if (component === 'MuiButton') {
    props.size = 'large'
    props.color = props.label.toLowerCase() === 'clear faults' ? 'error' : 'primary'
  }

  if (component === 'TextField') {
    props.size = 'medium'
  }

  if (component === 'MuiButton' || component === 'DataPoint' || component === 'TextField') {
    props.fullWidth = true
  }

  if (component === 'DataPoint') {
    props.variant = props.variant === 'dynamic' ? undefined : props.variant
  }

  return props
}

export const generateReactComponentFunction: (
  componentMetadata: SingleReactComponentMetadata,
  displayGroupID: string,
  componentID: string,
  uri?: string
) => ConfigurableComponentFunction = (componentMetadata, displayGroupID, componentID, uri) => {
  const { component, props } = componentMetadata

  const Component = storybookComponents[component]

  // we can pass scalar as a prop for relevant controls
  let scalar: number | null = null
  if (props && 'scalar' in props) {
    scalar = props.scalar
    delete props.scalar
  }

  const controlHandlerObject = uri ? generateControlHandlerObject(uri, component, scalar) : {}

  let memoized: MemoizedComponentObject | undefined = undefined

  return (assetState: ConfigurablePageStateStructure): JSX.Element => {
    const stateInfo = getSpecificStateInfo(assetState, displayGroupID, componentID, !!uri)

    if (memoized !== undefined && memoIsValid(memoized, stateInfo)) {
      return memoized.element
    }

    const updatedProps = organizeProps(props, component, stateInfo)

    const element = (
      <Component
        key={`${displayGroupID}/${componentID}`}
        {...updatedProps}
        {...controlHandlerObject}
      />
    )

    memoized = { prevState: stateInfo, element }

    return element
  }
}
