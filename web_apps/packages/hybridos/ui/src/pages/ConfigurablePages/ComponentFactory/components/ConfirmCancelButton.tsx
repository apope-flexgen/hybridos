import { MuiButton, CheckXConfirm } from '@flexgen/storybook';
import { useState } from 'react';

export interface ConfirmCancelButtonProps {
  label: string;
  color: any;
  onClick: () => void;
  disabled: boolean;
}

const ConfirmCancelButton: React.FC<ConfirmCancelButtonProps> = ({
  label,
  color,
  onClick,
  disabled,
}: ConfirmCancelButtonProps) => {
  const [showCheckX, setShowCheckX] = useState<boolean>(false);

  const onCheck = () => {
    onClick();
    setShowCheckX(false);
  };

  return (
    <div style={{ display: 'flex', alignItems: 'center', gap: '16px' }}>
      <MuiButton
        disabled={disabled}
        color={color}
        label={label}
        fullWidth={!showCheckX}
        size="large"
        onClick={() => setShowCheckX(true)}
      />
      {showCheckX && (
        <CheckXConfirm disabled={disabled} onCheck={onCheck} onX={() => setShowCheckX(false)} />
      )}
    </div>
  );
};

export default ConfirmCancelButton;
