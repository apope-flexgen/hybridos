import { ThemeType } from '@flexgen/storybook';
import { dia, shapes } from '@joint/plus';

import { DiagramItem } from './diagramDashboard.types';

/**
 * Format a label for use in site diagrams dashboard
 *
 * @param _label Label string to format, can be undefined
 * @returns Properly formatted label string
 */
export function formatDiagramlabel(_label?: string) {
  if (!_label) {
    return 'No label found';
  }
  let label = _label;
  label = label.replaceAll('psm_', '');
  label = label.replaceAll('_', ' ');
  label = label.replaceAll(/(?<!\w)ess/g, 'ESS');
  label = label.replaceAll(/(?<!\w)poi/g, 'POI');
  label = label.replaceAll(/(?<!\w)pv/g, 'PV');
  label = label.replace(/\b\w/g, (char: string) => char.toUpperCase()); // toTitleCase
  if (label.length > 8) {
    label = label.replaceAll(' ', '\n');
  }
  return label;
}

/**
 * Adds a DiagramItem to a JointJS Graph. Recursively calls itself on node.children[]
 *
 * @param node A diagram item which will be added to the graph
 * @param graph The jointjs graph which will have the node added to it.
 * @returns the ID of the shape which was just added to the graph
 */
export function addItemToGraph(node: DiagramItem, graph: dia.Graph, theme: ThemeType) {
  // static for now. Dynamic later.
  const size = { width: 100, height: 50 };

  const label = formatDiagramlabel(node?.id);
  const me = new shapes.standard.Rectangle({
    size,
    attrs: {
      label: {
        text: label,
        fill: theme.fgd.text.primary,
      },
      root: {
        cursor: 'auto',
      },
      body: {
        fill: theme.fgd.background.paper,
        stroke: theme.fgd.other.outline_border,
        rx: 5,
        ry: 5,
      },
    },
  });

  graph.addCell(me);
  (node?.children || []).forEach((child) => {
    const newShapeId = addItemToGraph(child, graph, theme);
    const link = new shapes.standard.Link({
      attrs: { wrapper: { cursor: 'auto' } },
      source: {
        id: me.id,
        port: 'out1',
        anchor: { name: 'center', args: { dx: size.width / 2, dy: size.height / 2 } },
      },
      target: { id: newShapeId, port: 'in1', anchor: { name: 'center', args: { dx: size.width / 2, dy: size.height / 2 } } },
    });
    graph.addCell(link);
  });
  return me.id;
}
