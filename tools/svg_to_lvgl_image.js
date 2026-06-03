const fs = require("fs");
const path = require("path");

function parseArgs(argv) {
  const args = {};
  for (let i = 2; i < argv.length; i += 2) {
    const key = argv[i];
    const value = argv[i + 1];
    if (!key || !key.startsWith("--") || value === undefined) {
      throw new Error("Invalid arguments");
    }
    args[key.slice(2)] = value;
  }
  return args;
}

function attrMap(tag) {
  const attrs = {};
  const re = /([a-zA-Z_:][-a-zA-Z0-9_:.]*)="([^"]*)"/g;
  let match;
  while ((match = re.exec(tag)) !== null) {
    attrs[match[1]] = match[2];
  }
  return attrs;
}

function parseStyleMap(svg) {
  const styles = {};
  const styleMatch = svg.match(/<style[^>]*>([\s\S]*?)<\/style>/);
  if (!styleMatch) {
    return styles;
  }

  const classRe = /([^{}]+)\{([^}]*)\}/g;
  let classMatch;
  while ((classMatch = classRe.exec(styleMatch[1])) !== null) {
    const values = {};
    for (const rule of classMatch[2].split(";")) {
      const [key, value] = rule.split(":").map((item) => item.trim());
      if (key && value) {
        values[key] = value;
      }
    }
    for (const selector of classMatch[1].split(",")) {
      const selectorMatch = selector.trim().match(/^\.([a-zA-Z0-9_-]+)$/);
      if (!selectorMatch) {
        continue;
      }
      styles[selectorMatch[1]] = {
        ...(styles[selectorMatch[1]] || {}),
        ...values,
      };
    }
  }
  return styles;
}

function parseColor(value) {
  if (!value || value === "none") {
    return null;
  }
  const hex = value.trim().replace("#", "");
  if (hex.length === 3) {
    return {
      r: parseInt(hex[0] + hex[0], 16),
      g: parseInt(hex[1] + hex[1], 16),
      b: parseInt(hex[2] + hex[2], 16),
    };
  }
  if (hex.length !== 6) {
    throw new Error(`Unsupported color: ${value}`);
  }
  return {
    r: parseInt(hex.slice(0, 2), 16),
    g: parseInt(hex.slice(2, 4), 16),
    b: parseInt(hex.slice(4, 6), 16),
  };
}

function multiply(left, right) {
  return [
    left[0] * right[0] + left[2] * right[1],
    left[1] * right[0] + left[3] * right[1],
    left[0] * right[2] + left[2] * right[3],
    left[1] * right[2] + left[3] * right[3],
    left[0] * right[4] + left[2] * right[5] + left[4],
    left[1] * right[4] + left[3] * right[5] + left[5],
  ];
}

function parseNumbers(text) {
  const result = [];
  const re = /[-+]?(?:\d*\.\d+|\d+\.?)(?:[eE][-+]?\d+)?/g;
  let match;
  while ((match = re.exec(text)) !== null) {
    result.push(Number(match[0]));
  }
  return result;
}

function parseTransform(text) {
  let matrix = [1, 0, 0, 1, 0, 0];
  if (!text) {
    return matrix;
  }

  const re = /([a-zA-Z]+)\(([^)]*)\)/g;
  let match;
  while ((match = re.exec(text)) !== null) {
    const name = match[1];
    const values = parseNumbers(match[2]);
    let next = [1, 0, 0, 1, 0, 0];
    if (name === "translate") {
      next = [1, 0, 0, 1, values[0] || 0, values[1] || 0];
    } else if (name === "rotate") {
      const angle = ((values[0] || 0) * Math.PI) / 180;
      const cos = Math.cos(angle);
      const sin = Math.sin(angle);
      next = [cos, sin, -sin, cos, 0, 0];
    } else if (name === "scale") {
      const sx = values[0] || 1;
      const sy = values.length > 1 ? values[1] : sx;
      next = [sx, 0, 0, sy, 0, 0];
    } else if (name === "matrix") {
      next = values.slice(0, 6);
    } else {
      throw new Error(`Unsupported transform: ${name}`);
    }
    matrix = multiply(matrix, next);
  }
  return matrix;
}

function applyMatrix(point, matrix) {
  return {
    x: point.x * matrix[0] + point.y * matrix[2] + matrix[4],
    y: point.x * matrix[1] + point.y * matrix[3] + matrix[5],
  };
}

function cubicPoint(p0, p1, p2, p3, t) {
  const u = 1 - t;
  const a = u * u * u;
  const b = 3 * u * u * t;
  const c = 3 * u * t * t;
  const d = t * t * t;
  return {
    x: a * p0.x + b * p1.x + c * p2.x + d * p3.x,
    y: a * p0.y + b * p1.y + c * p2.y + d * p3.y,
  };
}

function quadraticPoint(p0, p1, p2, t) {
  const u = 1 - t;
  return {
    x: u * u * p0.x + 2 * u * t * p1.x + t * t * p2.x,
    y: u * u * p0.y + 2 * u * t * p1.y + t * t * p2.y,
  };
}

function arcPoints(start, rx, ry, angle, largeArc, sweep, end) {
  if (rx === 0 || ry === 0) {
    return [end];
  }

  const phi = (angle * Math.PI) / 180;
  const cosPhi = Math.cos(phi);
  const sinPhi = Math.sin(phi);
  const dx = (start.x - end.x) / 2;
  const dy = (start.y - end.y) / 2;
  let x1p = cosPhi * dx + sinPhi * dy;
  let y1p = -sinPhi * dx + cosPhi * dy;
  rx = Math.abs(rx);
  ry = Math.abs(ry);

  const lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
  if (lambda > 1) {
    const scale = Math.sqrt(lambda);
    rx *= scale;
    ry *= scale;
  }

  const rx2 = rx * rx;
  const ry2 = ry * ry;
  const x1p2 = x1p * x1p;
  const y1p2 = y1p * y1p;
  const sign = largeArc === sweep ? -1 : 1;
  const coef = sign * Math.sqrt(Math.max(
      0, (rx2 * ry2 - rx2 * y1p2 - ry2 * x1p2) /
          (rx2 * y1p2 + ry2 * x1p2)));
  const cxp = coef * ((rx * y1p) / ry);
  const cyp = coef * (-(ry * x1p) / rx);
  const cx = cosPhi * cxp - sinPhi * cyp + (start.x + end.x) / 2;
  const cy = sinPhi * cxp + cosPhi * cyp + (start.y + end.y) / 2;

  function vectorAngle(ux, uy, vx, vy) {
    const dot = ux * vx + uy * vy;
    const len = Math.sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
    const signValue = ux * vy - uy * vx < 0 ? -1 : 1;
    return signValue * Math.acos(Math.max(-1, Math.min(1, dot / len)));
  }

  const ux = (x1p - cxp) / rx;
  const uy = (y1p - cyp) / ry;
  const vx = (-x1p - cxp) / rx;
  const vy = (-y1p - cyp) / ry;
  let startAngle = vectorAngle(1, 0, ux, uy);
  let delta = vectorAngle(ux, uy, vx, vy);
  if (!sweep && delta > 0) {
    delta -= Math.PI * 2;
  } else if (sweep && delta < 0) {
    delta += Math.PI * 2;
  }

  const steps = Math.max(8, Math.ceil(Math.abs(delta) / (Math.PI / 16)));
  const result = [];
  for (let i = 1; i <= steps; ++i) {
    const theta = startAngle + (delta * i) / steps;
    const x = cx + rx * Math.cos(theta) * cosPhi -
        ry * Math.sin(theta) * sinPhi;
    const y = cy + rx * Math.cos(theta) * sinPhi +
        ry * Math.sin(theta) * cosPhi;
    result.push({x, y});
  }
  return result;
}

function tokenizePath(data) {
  return data.match(/[a-zA-Z]|[-+]?(?:\d*\.\d+|\d+\.?)(?:[eE][-+]?\d+)?/g) ||
      [];
}

function isCommand(token) {
  return /^[a-zA-Z]$/.test(token);
}

function parsePath(data, matrix) {
  const tokens = tokenizePath(data);
  const points = [];
  let i = 0;
  let command = "";
  let current = {x: 0, y: 0};
  let start = {x: 0, y: 0};
  let lastControl = null;

  function number() {
    return Number(tokens[i++]);
  }

  function push(point) {
    const next = applyMatrix(point, matrix);
    points.push(next);
    current = point;
  }

  while (i < tokens.length) {
    if (isCommand(tokens[i])) {
      command = tokens[i++];
    }
    const relative = command === command.toLowerCase();
    const cmd = command.toUpperCase();

    if (cmd === "M") {
      const x = number();
      const y = number();
      current = {
        x: relative ? current.x + x : x,
        y: relative ? current.y + y : y,
      };
      start = current;
      points.push(applyMatrix(current, matrix));
      command = relative ? "l" : "L";
      lastControl = null;
    } else if (cmd === "L") {
      const x = number();
      const y = number();
      push({x: relative ? current.x + x : x,
            y: relative ? current.y + y : y});
      lastControl = null;
    } else if (cmd === "H") {
      const x = number();
      push({x: relative ? current.x + x : x, y: current.y});
      lastControl = null;
    } else if (cmd === "V") {
      const y = number();
      push({x: current.x, y: relative ? current.y + y : y});
      lastControl = null;
    } else if (cmd === "C") {
      const p0 = current;
      const p1 = {x: number(), y: number()};
      const p2 = {x: number(), y: number()};
      const p3 = {x: number(), y: number()};
      if (relative) {
        p1.x += current.x;
        p1.y += current.y;
        p2.x += current.x;
        p2.y += current.y;
        p3.x += current.x;
        p3.y += current.y;
      }
      for (let step = 1; step <= 24; ++step) {
        points.push(applyMatrix(
            cubicPoint(p0, p1, p2, p3, step / 24), matrix));
      }
      current = p3;
      lastControl = p2;
    } else if (cmd === "S") {
      const p0 = current;
      const p1 = lastControl == null
          ? current
          : {x: current.x * 2 - lastControl.x,
             y: current.y * 2 - lastControl.y};
      const p2 = {x: number(), y: number()};
      const p3 = {x: number(), y: number()};
      if (relative) {
        p2.x += current.x;
        p2.y += current.y;
        p3.x += current.x;
        p3.y += current.y;
      }
      for (let step = 1; step <= 24; ++step) {
        points.push(applyMatrix(
            cubicPoint(p0, p1, p2, p3, step / 24), matrix));
      }
      current = p3;
      lastControl = p2;
    } else if (cmd === "Q") {
      const p0 = current;
      const p1 = {x: number(), y: number()};
      const p2 = {x: number(), y: number()};
      if (relative) {
        p1.x += current.x;
        p1.y += current.y;
        p2.x += current.x;
        p2.y += current.y;
      }
      for (let step = 1; step <= 24; ++step) {
        points.push(applyMatrix(
            quadraticPoint(p0, p1, p2, step / 24), matrix));
      }
      current = p2;
      lastControl = p1;
    } else if (cmd === "T") {
      const p0 = current;
      const p1 = lastControl == null
          ? current
          : {x: current.x * 2 - lastControl.x,
             y: current.y * 2 - lastControl.y};
      const p2 = {x: number(), y: number()};
      if (relative) {
        p2.x += current.x;
        p2.y += current.y;
      }
      for (let step = 1; step <= 24; ++step) {
        points.push(applyMatrix(
            quadraticPoint(p0, p1, p2, step / 24), matrix));
      }
      current = p2;
      lastControl = p1;
    } else if (cmd === "A") {
      const rx = number();
      const ry = number();
      const angle = number();
      const largeArc = number() !== 0;
      const sweep = number() !== 0;
      const x = number();
      const y = number();
      const end = {
        x: relative ? current.x + x : x,
        y: relative ? current.y + y : y,
      };
      for (const point of arcPoints(current, rx, ry, angle,
                                   largeArc, sweep, end)) {
        points.push(applyMatrix(point, matrix));
      }
      current = end;
      lastControl = null;
    } else if (cmd === "Z") {
      points.push(applyMatrix(start, matrix));
      current = start;
      lastControl = null;
    } else {
      throw new Error(`Unsupported path command: ${command}`);
    }
  }

  return points;
}

function pointInPolygon(point, polygon) {
  let inside = false;
  for (let i = 0, j = polygon.length - 1; i < polygon.length; j = i++) {
    const pi = polygon[i];
    const pj = polygon[j];
    const intersects = ((pi.y > point.y) !== (pj.y > point.y)) &&
        point.x < ((pj.x - pi.x) * (point.y - pi.y)) / (pj.y - pi.y) + pi.x;
    if (intersects) {
      inside = !inside;
    }
  }
  return inside;
}

function composite(pixel, color, alpha) {
  const sourceAlpha = alpha;
  const targetAlpha = pixel.a / 255;
  const outAlpha = sourceAlpha + targetAlpha * (1 - sourceAlpha);
  if (outAlpha <= 0) {
    pixel.r = 0;
    pixel.g = 0;
    pixel.b = 0;
    pixel.a = 0;
    return;
  }
  pixel.r = Math.round((color.r * sourceAlpha +
      pixel.r * targetAlpha * (1 - sourceAlpha)) / outAlpha);
  pixel.g = Math.round((color.g * sourceAlpha +
      pixel.g * targetAlpha * (1 - sourceAlpha)) / outAlpha);
  pixel.b = Math.round((color.b * sourceAlpha +
      pixel.b * targetAlpha * (1 - sourceAlpha)) / outAlpha);
  pixel.a = Math.round(outAlpha * 255);
}

function mapPoint(point, viewBox, width, height, padding) {
  const scale = Math.min((width - padding * 2) / viewBox.w,
                         (height - padding * 2) / viewBox.h);
  const drawWidth = viewBox.w * scale;
  const drawHeight = viewBox.h * scale;
  const offsetX = (width - drawWidth) / 2;
  const offsetY = (height - drawHeight) / 2;
  return {
    x: offsetX + (point.x - viewBox.x) * scale,
    y: offsetY + (point.y - viewBox.y) * scale,
  };
}

function parseLength(value) {
  if (value == null) {
    return 0;
  }
  const numbers = parseNumbers(String(value));
  return numbers.length > 0 ? numbers[0] : 0;
}

function shapeStyle(attrs, styles) {
  const className = attrs.class || "";
  const style = styles[className] || {};
  const fill = parseColor(attrs.fill || style.fill || "#000000");
  const stroke = parseColor(attrs.stroke || style.stroke || null);
  const strokeWidth =
      parseLength(attrs["stroke-width"] || style["stroke-width"]);
  const opacity = Number(attrs.opacity || style.opacity || 1);
  return {fill, stroke, strokeWidth, opacity};
}

function ellipsePoints(cx, cy, rx, ry, matrix) {
  const steps = 72;
  const points = [];
  for (let i = 0; i < steps; ++i) {
    const angle = (Math.PI * 2 * i) / steps;
    points.push(applyMatrix({
      x: cx + Math.cos(angle) * rx,
      y: cy + Math.sin(angle) * ry,
    }, matrix));
  }
  return points;
}

function addStyledShape(shapes, points, style) {
  if (style.fill != null && style.opacity > 0) {
    shapes.push({
      type: "fill",
      points,
      color: style.fill,
      opacity: style.opacity,
    });
  }
  if (style.stroke != null && style.strokeWidth > 0 &&
      style.opacity > 0) {
    shapes.push({
      type: "stroke",
      points,
      color: style.stroke,
      opacity: style.opacity,
      strokeWidth: style.strokeWidth,
    });
  }
}

function collectShapes(svg) {
  const styles = parseStyleMap(svg);
  const viewBoxValues = parseNumbers(svg.match(/viewBox="([^"]+)"/)[1]);
  const viewBox = {
    x: viewBoxValues[0],
    y: viewBoxValues[1],
    w: viewBoxValues[2],
    h: viewBoxValues[3],
  };
  const shapes = [];
  const tagRe = /<(path|rect|circle|ellipse|line)\b[^>]*>/g;
  let match;
  while ((match = tagRe.exec(svg)) !== null) {
    const tag = match[0];
    const attrs = attrMap(tag);
    const style = shapeStyle(attrs, styles);
    if (style.fill == null && style.stroke == null) {
      continue;
    }

    if (match[1] === "path") {
      const matrix = parseTransform(attrs.transform);
      addStyledShape(shapes, parsePath(attrs.d, matrix), style);
    } else if (match[1] === "rect") {
      const x = Number(attrs.x || 0);
      const y = Number(attrs.y || 0);
      const w = Number(attrs.width || 0);
      const h = Number(attrs.height || 0);
      const matrix = parseTransform(attrs.transform);
      addStyledShape(shapes, [
        applyMatrix({x, y}, matrix),
        applyMatrix({x: x + w, y}, matrix),
        applyMatrix({x: x + w, y: y + h}, matrix),
        applyMatrix({x, y: y + h}, matrix),
      ], style);
    } else if (match[1] === "circle") {
      const cx = Number(attrs.cx || 0);
      const cy = Number(attrs.cy || 0);
      const r = Number(attrs.r || 0);
      const matrix = parseTransform(attrs.transform);
      addStyledShape(shapes, ellipsePoints(cx, cy, r, r, matrix), style);
    } else if (match[1] === "ellipse") {
      const cx = Number(attrs.cx || 0);
      const cy = Number(attrs.cy || 0);
      const rx = Number(attrs.rx || 0);
      const ry = Number(attrs.ry || 0);
      const matrix = parseTransform(attrs.transform);
      addStyledShape(shapes, ellipsePoints(cx, cy, rx, ry, matrix), style);
    } else if (match[1] === "line") {
      const x1 = Number(attrs.x1 || 0);
      const y1 = Number(attrs.y1 || 0);
      const x2 = Number(attrs.x2 || 0);
      const y2 = Number(attrs.y2 || 0);
      const matrix = parseTransform(attrs.transform);
      addStyledShape(shapes, [
        applyMatrix({x: x1, y: y1}, matrix),
        applyMatrix({x: x2, y: y2}, matrix),
      ], {
        fill: null,
        stroke: style.stroke,
        strokeWidth: style.strokeWidth,
        opacity: style.opacity,
      });
    }
  }
  return {viewBox, shapes};
}

function distanceToSegment(point, start, end) {
  const dx = end.x - start.x;
  const dy = end.y - start.y;
  if (dx === 0 && dy === 0) {
    return Math.hypot(point.x - start.x, point.y - start.y);
  }
  const t = Math.max(0, Math.min(1,
      ((point.x - start.x) * dx + (point.y - start.y) * dy) /
      (dx * dx + dy * dy)));
  return Math.hypot(point.x - (start.x + t * dx),
                    point.y - (start.y + t * dy));
}

function strokeContains(point, points, strokeWidth) {
  const radius = strokeWidth / 2;
  for (let i = 1; i < points.length; ++i) {
    if (distanceToSegment(point, points[i - 1], points[i]) <= radius) {
      return true;
    }
  }
  return false;
}

function render(svg, width, height, padding) {
  const {viewBox, shapes} = collectShapes(svg);
  const pixels = Array.from({length: width * height}, () => ({
    r: 0, g: 0, b: 0, a: 0,
  }));
  const sampleCount = 4;

  for (const shape of shapes) {
    const polygon = shape.points.map(
        (point) => mapPoint(point, viewBox, width, height, padding));
    const scale = Math.min((width - padding * 2) / viewBox.w,
                           (height - padding * 2) / viewBox.h);
    const strokeWidth = shape.strokeWidth * scale;
    for (let y = 0; y < height; ++y) {
      for (let x = 0; x < width; ++x) {
        let coverage = 0;
        for (let sy = 0; sy < sampleCount; ++sy) {
          for (let sx = 0; sx < sampleCount; ++sx) {
            const point = {
              x: x + (sx + 0.5) / sampleCount,
              y: y + (sy + 0.5) / sampleCount,
            };
            const contains = shape.type === "stroke"
                ? strokeContains(point, polygon, strokeWidth)
                : pointInPolygon(point, polygon);
            if (contains) {
              coverage += 1;
            }
          }
        }
        if (coverage > 0) {
          composite(pixels[y * width + x], shape.color,
                    (shape.opacity * coverage) / (sampleCount * sampleCount));
        }
      }
    }
  }
  return pixels;
}

function cArray(bytes) {
  const lines = [];
  for (let i = 0; i < bytes.length; i += 8) {
    lines.push("    " + bytes.slice(i, i + 8)
        .map((value) => `0x${value.toString(16).padStart(2, "0")}`)
        .join(", ") + ",");
  }
  return lines.join("\n");
}

function localTimestamp() {
  const now = new Date();
  const pad = (value) => String(value).padStart(2, "0");
  return [
    now.getFullYear(),
    "-",
    pad(now.getMonth() + 1),
    "-",
    pad(now.getDate()),
    " ",
    pad(now.getHours()),
    ":",
    pad(now.getMinutes()),
    ":",
    pad(now.getSeconds()),
  ].join("");
}

function writeC(output, name, pixels, width, height) {
  const bytes = [];
  for (const pixel of pixels) {
    bytes.push(pixel.b, pixel.g, pixel.r, pixel.a);
  }
  const stamp = localTimestamp();
  const source = `/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: ${stamp}
 * @LastEditTime: ${stamp}
 * @License: GPL 3.0
 */
#include "lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMAGE_${name.toUpperCase()}
#define LV_ATTRIBUTE_IMAGE_${name.toUpperCase()}
#endif

static const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST
    LV_ATTRIBUTE_IMAGE_${name.toUpperCase()} uint8_t
    ${name}_map[] = {
${cArray(bytes)}
};

const lv_image_dsc_t ${name} = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_ARGB8888,
    .header.flags = 0,
    .header.w = ${width},
    .header.h = ${height},
    .header.stride = ${width * 4},
    .data_size = sizeof(${name}_map),
    .data = ${name}_map,
};
`;
  fs.mkdirSync(path.dirname(output), {recursive: true});
  fs.writeFileSync(output, source, "utf8");
}

function main() {
  const args = parseArgs(process.argv);
  const input = args.input;
  const output = args.output;
  const name = args.name;
  const width = Number(args.width);
  const height = Number(args.height);
  const padding = Number(args.padding || 0);
  if (!input || !output || !name || !width || !height) {
    throw new Error("Required: --input --output --name --width --height");
  }
  const svg = fs.readFileSync(input, "utf8");
  const pixels = render(svg, width, height, padding);
  writeC(output, name, pixels, width, height);
}

main();
