<script setup lang="ts">
import { ref } from 'vue';
import type { Ref } from 'vue';

const xAxis = ref(0);
const yAxis = ref(0);
const iterations = ref(0);
const currentIteration = ref(0);
const duration = ref(0);

enum PositionType {
  Normal,
  Water,
  Sand,
  Rock,
}

type TPosition = {
  x: number;
  y: number;
  type: PositionType;
  height: number;
};

type TMap = {
  iteration: number;
  positions: TPosition[][];
};
const mapLayer: Ref<TMap> = ref([]);

enum CellType {
  DeadCell,
  NormalCell,
  WaterCell,
  SandCell,
  Predator,
  NormalParasite,
  WaterParasite,
  SandParasite,
  BigPredator,
  BigCell,
}

interface ICell {
  x: number;
  y: number;
  type: CellType;
  age: number;
};

class Cell implements ICell {
  x: number;
  y: number;
  type: CellType;
  age: number;
  constructor(x: number, y: number, type: CellType, age: number) {
    this.x = x;
    this.y = y;
    this.type = type;
    this.age = age;
  }

  setDeadCell = () => {
    this.type = CellType.DeadCell;
    this.age = 0;
  };

  get neighbourStatus() {
    let deadCellCount = 0;
    let cellCount = 0;
    let predatorCount = 0;
    const neighbours: ICell[] = [];
    for (let i = this.x - 1; i <= this.x + 1; i++) {
      if (i < 0 || i >= xAxis.value) continue;
      for (let j = this.y - 1; j <= this.y + 1; j++) {
        if (j < 0 || j >= yAxis.value) continue;
        if (i === this.x && j === this.y) continue;
        neighbours.push({
          x: i,
          y: j,
          type: mapLayer.value.positions[i][j].type,
          age: mapLayer.value.positions[i][j].height,
        });
        if (mapLayer.value.positions[i][j].type === CellType.DeadCell) {
          deadCellCount++;
        } else if (mapLayer.value.positions[i][j].type === CellType.Predator || mapLayer.value.positions[i][j].type === CellType.BigPredator) {
          predatorCount++;
        } else {
          cellCount++;
        }
      }
    }
    return {
      neighbours,
      deadCellCount,
      cellCount,
      predatorCount,
    };
  }
}

function createMap(xAxis: number, yAxis: number, iteration: number) {
  const map: TMap[] = [];
  for (let i = 0; i < iteration; i++ ) {
    const layer: TMap = {
      iteration: i,
      positions: [],
    };
    const context: TPosition[][] = [];
    for (let j = 0; j < xAxis; j++) {
      const row: TPosition[] = [];
      for (let k = 0; k < yAxis; k++) {
        row.push({
          x: j,
          y: k,
          type: PositionType.Normal,
          height: 0,
        });
      }
      context.push(row);
    }
    layer.positions = context;
  }
  return map;
}

function modifyMapElement(item: TPosition, type: PositionType, height = 0) {
  item.type = type;
  item.height = height;
}

function modifyCell(item: ICell, type: CellType, age = 0) {
  item.type = type;
  item.age = age;
}

</script>

<template>
  <div class="bg-gray-50 h-screen">
    <div></div>
  </div>
</template>
