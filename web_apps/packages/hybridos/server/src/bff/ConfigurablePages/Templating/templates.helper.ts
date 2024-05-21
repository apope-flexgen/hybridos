import { Template } from '../SiteDiagram/siteDiagram.types';

export const generateTemplatedExtensions = (field: string, template: Template): string[] => {
  switch (template.type) {
    case 'list':
      return handleListTemplate(field, template);
    case 'range':
      return handleRangeTemplate(field, template);
    case 'sequential':
      return handleSequentialTemplate(field, template);
    default:
      return [];
  }
};

const handleListTemplate = (field: string, template: Template): string[] => {
  return template.list.map((listItem) => {
    return field.replace(template.token, listItem);
  });
};

const handleRangeTemplate = (field: string, template: Template): string[] => {
  const extensions = [];
  template.range.forEach((rangeItem) => {
    if (typeof rangeItem === 'string' && rangeItem.includes('..')) {
      const arrayOfRange = rangeItem.split('..');
      const startNumber = Number(arrayOfRange[0]);
      const endNumber = Number(arrayOfRange[1]);
      for (var i = startNumber; i < endNumber + 1; i++) {
        const fullExtension = String(i).padStart(template.minWidth || 1, '0');
        extensions.push(field.replace(template.token, fullExtension));
      }
    } else {
      const fullExtension = String(rangeItem).padStart(template.minWidth || 1, '0');
      extensions.push(field.replace(template.token, fullExtension));
    }
  });
  return extensions;
};

const handleSequentialTemplate = (field: string, template: Template): string[] => {
  const extensions = [];
  for (var i = template.from; i <= template.to; i++) {
    const fullExtension = String(i).padStart(template.minWidth || 1, '0');
    extensions.push(field.replace(template.token, fullExtension));
  }
  return extensions;
};
