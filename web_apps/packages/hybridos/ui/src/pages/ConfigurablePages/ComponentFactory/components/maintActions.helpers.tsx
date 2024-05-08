import { Step } from '@flexgen/storybook';
import React from 'react';
import { MaintenanceActionStep } from 'shared/types/dtos/configurablePages.dto';

export enum MaintenanceActionStatuses {
  Aborted = 'aborted',
  Failed = 'failed',
  Completed = 'completed',
  InProgress = 'in progress',
  Exiting = 'exiting',
}

export const convertFromMilliseconds = (duration: number) => {
  if (!duration) return 'unknown';
  const minutes = Math.floor(duration / 60000);
  const seconds = ((duration % 60000) / 1000).toFixed(0);
  return minutes < 1 ? `${seconds} seconds` : `${minutes} minutes, ${seconds} seconds`;
};

export const createStepComponents = (
  steps: MaintenanceActionStep[],
  stepIndex: number,
  status: MaintenanceActionStatuses,
  orientation: 'horizontal' | 'vertical',
) => steps.map((step, index) => (
  <Step
    label={orientation === 'vertical' ? step.step_name : ''}
    subheader={
        orientation === 'vertical'
          ? `Estimated duration: ${convertFromMilliseconds(step.estimated_duration)}`
          : ''
      }
    orientation={orientation}
    inProgress={
        index === stepIndex
        && (status === MaintenanceActionStatuses.InProgress
          || status === MaintenanceActionStatuses.Aborted)
      }
    completed={index < stepIndex || status === MaintenanceActionStatuses.Completed}
    error={index === stepIndex && status === MaintenanceActionStatuses.Failed}
    showStepConnector={!!(orientation === 'vertical' && index !== steps.length - 1)}
  />
));

export const decideStatusIcon = (status: MaintenanceActionStatuses) => {
  if (
    status === MaintenanceActionStatuses.InProgress
    || status === MaintenanceActionStatuses.Exiting
  ) return 'Loading';
  if (status === MaintenanceActionStatuses.Failed) return 'InfoOutline';
  if (status === MaintenanceActionStatuses.Aborted) return 'Close';
  return 'Check';
};

export const decideStatusIconColor = (status: MaintenanceActionStatuses) => {
  if (
    status === MaintenanceActionStatuses.InProgress
    || status === MaintenanceActionStatuses.Exiting
  ) return 'primary';
  if (status === MaintenanceActionStatuses.Failed || status === MaintenanceActionStatuses.Aborted) return 'error';
  return 'success';
};

export const decideStepCountText = (stepIndex: number, steps: MaintenanceActionStep[]) => `Step ${stepIndex === steps.length ? stepIndex : stepIndex + 1} of ${steps.length}`;

export const cardContainerSx = {
  boxShadow: 'none',
  gap: '12px',
  border: 'solid 1px #EAECF0',
  padding: '16px',
};
export const contentBoxSx = {
  display: 'flex',
  justifyContent: 'space-between',
  flex: 1,
  alignItems: 'flex-start',
};
export const textBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'flex-start',
  gap: '6px',
};
export const stepperBoxSx = { display: 'flex', alignItems: 'center' };
export const modalContentSx = {
  width: '100%',
  display: 'flex',
  flexDirection: 'column',
  gap: '12px',
  padding: '0 24px 24px 24px',
};
export const buttonBoxSx = { display: 'flex', gap: '12px' };
export const halfWidth = { width: '50%' };
