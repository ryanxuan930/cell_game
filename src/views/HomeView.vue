<script setup lang="ts">
import { ref, computed, onMounted } from 'vue';
import type { Ref } from 'vue';
import InputNumber from 'primevue/inputnumber';
import Button from 'primevue/button';
import Dialog from 'primevue/dialog';
import Dropdown from 'primevue/dropdown';

const xAxis = ref(16);
const yAxis = ref(32);
const iterations = ref(30);
const currentIteration = ref(0);
const duration = ref(1);
let interval: number;
const randomMap = ref(false);
const randomCell = ref(false);
const showModal = ref(false);
const isLoading = ref(false);

const elementSize = ref('20px')

enum PositionType {
  Normal,
  Water,
  Sand,
  Rock,
}

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

enum WeatherType {
  Normal,
  Rain,
  Sunlight,
}

const currentWeather: Ref<WeatherType> = ref(WeatherType.Normal);

const weatherData: Ref<WeatherType[]> = computed(() => {
  const weather: WeatherType[] = [];
  const num = Math.ceil(iterations.value / 10);
  for (let i = 0; i < num; i++) {
    if (i < 3) {
      switch (i) {
        case 0:
          weather.push(WeatherType.Normal);
          break;
        case 1:
          weather.push(WeatherType.Rain);
          break;
        case 2:
          weather.push(WeatherType.Sunlight);
          break;
        default:
          break;
      }
    } else {
      const random = Math.random();
      if (random < 0.3) {
        weather.push(WeatherType.Rain);
      } else if (random < 0.6) {
        weather.push(WeatherType.Sunlight);
      } else {
        weather.push(WeatherType.Normal);
      }
    }
  }
  return weather;
})

const cellTypeList = [
  { label: '無細胞', value: CellType.DeadCell },
  { label: '一般細胞', value: CellType.NormalCell },
  { label: '大型細胞', value: CellType.BigCell },
  { label: '水域細胞', value: CellType.WaterCell },
  { label: '沙漠細胞', value: CellType.SandCell },
  { label: '掠食者', value: CellType.Predator },
  { label: '大型掠食者', value: CellType.BigPredator },
  { label: '一般寄生細胞', value: CellType.NormalParasite },
  { label: '水域寄生細胞', value: CellType.WaterParasite },
  { label: '沙漠寄生細胞', value: CellType.SandParasite },
];

const positionTypeList = [
  { label: '一般', value: PositionType.Normal },
  { label: '水域', value: PositionType.Water },
  { label: '沙漠', value: PositionType.Sand },
  { label: '岩石', value: PositionType.Rock },
];

interface ICell {
  x: number;
  y: number;
  type: CellType;
  age: number;
  resetAge: () => void;
  neighbourStatus: (iteration: number) => {
    neighbours: TPosition[];
    deadCellCount: number;
    cellCount: number;
    predatorCount: number;
    parasiteCount: number;
    neighborCount: number;
    sandCellCount: number;
    bigCellCount: number;
  };
};

type TPosition = {
  x: number;
  y: number;
  type: PositionType;
  height: number;
  content: ICell;
};

type TMap = {
  iteration: number;
  positions: TPosition[][];
};
const mapLayer: Ref<TMap[]> = ref([]);

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

  resetAge = () => {
    this.age = 0;
  };

  neighbourStatus = (iteration: number) => {
    let deadCellCount = 0;
    let cellCount = 0;
    let predatorCount = 0;
    let parasiteCount = 0;
    let neighborCount = 0;
    let sandCellCount = 0;
    let bigCellCount = 0;
    const neighbours: TPosition[] = [];
    for (let i = this.x - 1; i <= this.x + 1; i++) {
      if (i < 0 || i >= xAxis.value) continue;
      for (let j = this.y - 1; j <= this.y + 1; j++) {
        if (j < 0 || j >= yAxis.value) continue;
        if (i === this.x && j === this.y) continue;
        neighbours.push({
          x: i,
          y: j,
          type: mapLayer.value[iteration].positions[i][j].type,
          height: mapLayer.value[iteration].positions[i][j].height,
          content: mapLayer.value[iteration].positions[i][j].content,
        });
        if (mapLayer.value[iteration].positions[i][j].content.type === CellType.DeadCell) {
          deadCellCount++;
        } else if (mapLayer.value[iteration].positions[i][j].content.type === CellType.Predator || mapLayer.value[iteration].positions[i][j].content.type === CellType.BigPredator) {
          predatorCount++;
        } else if (mapLayer.value[iteration].positions[i][j].content.type === CellType.NormalParasite && mapLayer.value[iteration].positions[i][j].content.type === CellType.WaterParasite && mapLayer.value[iteration].positions[i][j].content.type === CellType.SandParasite) {
          parasiteCount++;
          neighborCount++;
          if (mapLayer.value[iteration].positions[i][j].content.type === CellType.SandCell) {
            sandCellCount++;
          } else if (mapLayer.value[iteration].positions[i][j].content.type === CellType.BigCell) {
            bigCellCount++;
          }
        } else {
          cellCount++;
          neighborCount++;
        }
      }
    }
    return {
      neighbours,
      deadCellCount,
      cellCount,
      predatorCount,
      parasiteCount,
      neighborCount,
      sandCellCount,
      bigCellCount,
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
          content: new Cell(j, k, CellType.DeadCell, 0),
        });
      }
      context.push(row);
    }
    layer.positions = context;
    map.push(layer);
  }
  return map;
}

function initializeMap(randomMap: boolean = false, randomCell: boolean = false) {
  isLoading.value = true;
  mapLayer.value = createMap(xAxis.value, yAxis.value, iterations.value);
  currentIteration.value = 0;
  if (randomMap || randomCell) {
    for (let i = 0; i < xAxis.value; i++) {
      for (let j = 0; j < yAxis.value; j++) {
        if (randomMap) {
          let random = 0;
          for (let k = 0; k < iterations.value; k++) {
            if (k % 10 === 0) {
              random = Math.random();
            }
            if ( k < 10) {
              if (random < 0.1) {
                modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Rock);
              } else if (random < 0.3) {
                modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Water);
              } else if (random < 0.7) {
                modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Sand);
              } else {
                modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Normal);
              }
            } else {
              if (mapLayer.value[k-1].positions[i][j].type === PositionType.Rock) {
                modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Rock);
              } else {
                const weatherIndex: number = Math.floor(k / 10);
                if (weatherData.value[weatherIndex] === WeatherType.Rain) {
                  if (mapLayer.value[k].positions[i][j].type === PositionType.Sand) {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Normal);
                  } else {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Water);
                  }
                  if (random < 0.1 && mapLayer.value[k].positions[i][j].type === PositionType.Water) {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Sand);
                  }
                } else if (weatherData.value[weatherIndex] === WeatherType.Sunlight) {
                  if (mapLayer.value[k].positions[i][j].type === PositionType.Water) {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Normal);
                  } else {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Sand);
                  }
                  if (random < 0.1 && mapLayer.value[k].positions[i][j].type === PositionType.Sand) {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Water);
                  }
                } else {
                  if (random < 0.3) {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Water);
                  } else if (random < 0.7) {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Sand);
                  } else {
                    modifyMapElement(mapLayer.value[k].positions[i][j], PositionType.Normal);
                  }
                }
              }
            }
          }
        }
        if (randomCell && mapLayer.value[0].positions[i][j].type !== PositionType.Rock) {
          const random = Math.random();
          if (random < 0.05) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.NormalCell);
          } else if (random < 0.1) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.WaterCell);
          } else if (random < 0.15) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.SandCell);
          } else if (random < 0.2) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.Predator);
          } else if (random < 0.25) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.NormalParasite);
          } else if (random < 0.3) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.WaterParasite);
          } else if (random < 0.35) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.SandParasite);
          } else if (random < 0.4) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.BigPredator);
          } else if (random < 0.45) {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.BigCell);
          } else {
            modifyCell(mapLayer.value[0].positions[i][j].content, CellType.DeadCell);
          }
        }
      }
    }
  }
  isLoading.value = false;
}

function modifyMapElement(item: TPosition, type: PositionType, height = 0) {
  item.type = type;
  item.height = height;
}

function modifyCell(item: ICell, type: CellType, age = 0) {
  item.type = type;
  item.age = age;
}

function movePositionToNextIteration(prevItem: TPosition, nextItem: TPosition) {
  nextItem.type = prevItem.type;
  nextItem.height = prevItem.height;
}

function moveToNextIteration(prevItem: TPosition, nextItem: TPosition) {
  nextItem.type = prevItem.type;
  nextItem.height = prevItem.height;
  nextItem.content = prevItem.content;
  nextItem.content.age++;
}

function cellBackgroundHandler(type: PositionType) {
  switch (type) {
    case PositionType.Normal:
      return 'bg-green-100';
    case PositionType.Water:
      return 'bg-blue-100';
    case PositionType.Sand:
      return 'bg-yellow-100';
    case PositionType.Rock:
      return 'bg-gray-600';
    default:
      return 'bg-gray-600';
  }
}

function cellColorHandler(type: CellType) {
  switch (type) {
    case CellType.DeadCell:
      return 'bg-transparent';
    case CellType.NormalCell:
      return 'bg-green-500';
    case CellType.WaterCell:
      return 'bg-blue-500';
    case CellType.SandCell:
      return 'bg-yellow-500';
    case CellType.Predator:
      return 'bg-red-500';
    case CellType.NormalParasite:
      return 'bg-green-300';
    case CellType.WaterParasite:
      return 'bg-blue-300'; 
    case CellType.SandParasite:
      return 'bg-yellow-300';
    case CellType.BigPredator:
      return 'bg-red-300';
    case CellType.BigCell:
      return 'bg-gray-300';
    default:
      return 'bg-gray-600';
  }
}

const selectedCell: Ref<TPosition> = ref({
  x: 0,
  y: 0,
  type: PositionType.Normal,
  height: 0,
  content: {
    x: 0,
    y: 0,
    type: CellType.DeadCell,
    age: 0,
    resetAge: () => {},
    neighbourStatus: (iteration: number) => {
      return {
        neighbours: [],
        deadCellCount: 0,
        cellCount: 0,
        predatorCount: 0,
        parasiteCount: 0,
        neighborCount: 0,
        sandCellCount: 0,
        bigCellCount: 0,
      };
    },
  },
});

function deepCopy(obj: any) {
  if (typeof obj === 'object' && obj !== null) {
    const copy = Array.isArray(obj) ? [] : {};
    for (const key in obj) {
        copy[key] = deepCopy(obj[key]);
    }
    return copy;
  } else if (typeof obj === 'function') {
    return obj.bind(this);
  }
  return obj;
}

function cellHandler(input: TPosition) {
  selectedCell.value = deepCopy(input);
  showModal.value = true;
}

function saveCellEdit() {
  mapLayer.value[currentIteration.value].positions[selectedCell.value.x][selectedCell.value.y] = selectedCell.value;
  showModal.value = false;
}

const isPlaying = ref(false);

function startGame() {
  isPlaying.value = true;
  currentIteration.value = 0;
  interval = setInterval(async () => {
    if (currentIteration.value < iterations.value - 1) {
      currentIteration.value++;
      if (currentIteration.value % 10 === 0) {
        currentWeather.value = weatherData.value[Math.floor(currentIteration.value / 10)];
      }
    } else {
      isPlaying.value = false;
      clearInterval(interval);
      currentIteration.value = 0;
      currentWeather.value = WeatherType.Normal;
    }
  }, duration.value *1000);
}

function stopGame() {
  isPlaying.value = false;
  clearInterval(interval);
  currentIteration.value = 0;
}

function CalculateEvolution() {
  isLoading.value = true;
  for (let i = 0; i < iterations.value - 1; i++) {
    for (let x = 0; x < xAxis.value; x++) {
      for (let y = 0; y < yAxis.value; y++) {
        mapLayer.value[i].positions[x][y].content.resetAge();
      }
    }
  }
  for (let i = 0; i < iterations.value - 1; i++) {
    for (let x = 0; x < xAxis.value; x++) {
      for (let y = 0; y < yAxis.value; y++) {
        const currentCell = mapLayer.value[i].positions[x][y];
        if (currentCell.type === PositionType.Rock) {
          moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
        } else {
          const neighbours = currentCell.content.neighbourStatus(i);
          // Normal
          if (currentCell.type === PositionType.Normal) {
            if (
              currentCell.content.type === CellType.DeadCell
              && neighbours.neighborCount === 3
            ) {
              modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.NormalCell);
              movePositionToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
            } else if (
              currentCell.content.type === CellType.NormalCell 
              || currentCell.content.type === CellType.WaterCell 
              || currentCell.content.type === CellType.SandCell 
              || currentCell.content.type === CellType.BigCell
              ) {
              if (neighbours.neighborCount !== 3) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (currentCell.content.type === CellType.Predator) {
              if (neighbours.predatorCount > 0) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              } 
            } else if (currentCell.content.type === CellType.BigPredator) {
              if (neighbours.predatorCount > 1) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (
              currentCell.content.type === CellType.NormalParasite 
              || currentCell.content.type === CellType.WaterParasite 
              || currentCell.content.type === CellType.SandParasite
              ) {
              if (neighbours.neighborCount !== 2) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else {
              moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
            }
          // Water
          } else if (currentCell.type === PositionType.Water) {
            if (
              currentCell.content.type === CellType.DeadCell 
              && neighbours.neighborCount > 0 
              && neighbours.neighborCount < 5
              ) {
              modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.NormalCell);
            } else if (
              currentCell.content.type === CellType.NormalCell 
              || currentCell.content.type === CellType.WaterCell 
              || currentCell.content.type === CellType.SandCell 
              || currentCell.content.type === CellType.BigCell
              ) {
              if (neighbours.neighborCount < 2 && neighbours.neighborCount > 3) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (currentCell.content.type === CellType.Predator) {
              if (neighbours.predatorCount > 0) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (currentCell.content.type === CellType.BigPredator) {
              if (neighbours.predatorCount > 1) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (
              currentCell.content.type === CellType.NormalParasite 
              || currentCell.content.type === CellType.WaterParasite 
              || currentCell.content.type === CellType.SandParasite
              ) {
              if (neighbours.neighborCount !== 2) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            }
          // Sand
          } else if (currentCell.type === PositionType.Sand) {
            if (currentCell.content.type === CellType.DeadCell && neighbours.neighborCount === 4) {
              modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.NormalCell);
            } else if (currentCell.content.type === CellType.NormalCell) {
              if (neighbours.neighborCount !== 2) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else if (i > 1) {
                if (
                  mapLayer.value[i].positions[x][y].content.type === CellType.NormalCell
                  && mapLayer.value[i - 1].positions[x][y].content.type === CellType.NormalCell
                  && mapLayer.value[i - 2].positions[x][y].content.type === CellType.NormalCell
                  && (
                    mapLayer.value[i].positions[x][y].content.neighbourStatus(i).sandCellCount + mapLayer.value[i].positions[x][y].content.neighbourStatus(i).bigCellCount > 0 
                    || mapLayer.value[i - 1].positions[x][y].content.neighbourStatus(i).sandCellCount + mapLayer.value[i - 1].positions[x][y].content.neighbourStatus(i).bigCellCount > 0 
                    || mapLayer.value[i - 2].positions[x][y].content.neighbourStatus(i).sandCellCount + mapLayer.value[i - 2].positions[x][y].content.neighbourStatus(i).bigCellCount > 0
                    )
                  ) {
                  modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.SandCell);
                } else {
                  modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
                }
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (currentCell.content.type === CellType.WaterCell) {
              if (neighbours.neighborCount !== 2) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                if (i > 2) {
                  if (mapLayer.value[i].positions[x][y].content.type === CellType.WaterCell && mapLayer.value[i - 1].positions[x][y].content.type === CellType.WaterCell && mapLayer.value[i - 2].positions[x][y].content.type === CellType.WaterCell) {
                    modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
                  } else {
                    moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
                  }
                } else {
                  moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
                }
              }
            } else if (currentCell.content.type === CellType.SandCell) {
              if (neighbours.neighborCount !== 2) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (currentCell.content.type === CellType.Predator) {
              if (neighbours.predatorCount > 0) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                if (i > 2) {
                  if (mapLayer.value[i].positions[x][y].content.type === CellType.Predator && mapLayer.value[i - 1].positions[x][y].content.type === CellType.Predator && mapLayer.value[i - 2].positions[x][y].content.type === CellType.Predator) {
                    modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
                  } else {
                    moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
                  }
                } else {
                  moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
                }
              }
            } else if (currentCell.content.type === CellType.BigPredator) {
              if (neighbours.predatorCount > 1) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else if (i > 1 
                && currentCell.content.type === CellType.BigPredator 
                && mapLayer.value[i - 1].positions[x][y].content.type === CellType.BigPredator
                && mapLayer.value[i - 2].positions[x][y].content.type === CellType.BigPredator
                ) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (
                currentCell.content.type === CellType.NormalParasite 
                || currentCell.content.type === CellType.WaterParasite 
                || currentCell.content.type === CellType.SandParasite
              ) {
              if (neighbours.neighborCount !== 2) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else if (i > 0 
                && (
                  currentCell.content.type === CellType.NormalParasite 
                  || currentCell.content.type === CellType.WaterParasite 
                  || currentCell.content.type === CellType.SandParasite
                ) && (
                  mapLayer.value[i - 1].positions[x][y].content.type === CellType.NormalParasite
                  || mapLayer.value[i - 1].positions[x][y].content.type === CellType.WaterParasite
                  || mapLayer.value[i - 1].positions[x][y].content.type === CellType.SandParasite
                  )
                ) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            } else if (currentCell.content.type === CellType.BigCell) {
              if (neighbours.neighborCount !== 2) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else if (
                i > 1 
                && mapLayer.value[i - 1].positions[x][y].content.type === CellType.BigCell
                ) {
                modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.DeadCell);
              } else {
                moveToNextIteration(currentCell, mapLayer.value[i + 1].positions[x][y]);
              }
            }
          }
          if (
            currentCell.content.type === CellType.NormalCell 
            && currentCell.content.age % 3 === 2 
          ) {
            if (currentCell.type === PositionType.Normal) {
              modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.BigCell);
            } else if (currentCell.type === PositionType.Water) {
              modifyCell(mapLayer.value[i + 1].positions[x][y].content, CellType.WaterCell);
            }
          }
          if (
          (
            (
              currentCell.content.type === CellType.Predator 
              && currentCell.content.age % 3 === 2 
            ) || (
              currentCell.content.type === CellType.BigPredator 
              && currentCell.content.age % 5 === 4 
            )
          )
          && currentCell.content.neighbourStatus(i).predatorCount === 0 
          && currentCell.content.neighbourStatus(i).cellCount > 0
          ) {
            for (let item = 0; item < currentCell.content.neighbourStatus(i).neighbours.length; item++) {
              if (currentCell.content.neighbourStatus(i).neighbours[item].content.type !== CellType.DeadCell) {
                modifyCell(mapLayer.value[i + 1].positions[currentCell.content.neighbourStatus(i).neighbours[item].x][currentCell.content.neighbourStatus(i).neighbours[item].y].content, CellType.DeadCell);
              }
            }
          }
          if (
          (
            currentCell.content.type === CellType.NormalParasite
            || currentCell.content.type === CellType.WaterParasite
            || currentCell.content.type === CellType.SandParasite
          )
          && currentCell.content.neighbourStatus(i).predatorCount === 0 
          && currentCell.content.neighbourStatus(i).cellCount > 0
          ) {
            for (let item = 0; item < currentCell.content.neighbourStatus(i).neighbours.length; item++) {
              if (
                currentCell.content.neighbourStatus(i).neighbours[item].content.type !== CellType.DeadCell
                && currentCell.content.neighbourStatus(i).neighbours[item].content.type !== CellType.Predator
                && currentCell.content.neighbourStatus(i).neighbours[item].content.type !== CellType.BigPredator
                ) {
                switch (currentCell.content.neighbourStatus(i).neighbours[item].content.type) {
                  case CellType.NormalParasite:
                    modifyCell(mapLayer.value[i + 1].positions[currentCell.content.neighbourStatus(i).neighbours[item].x][currentCell.content.neighbourStatus(i).neighbours[item].y].content, CellType.NormalParasite);
                    break;
                  case CellType.WaterParasite:
                    modifyCell(mapLayer.value[i + 1].positions[currentCell.content.neighbourStatus(i).neighbours[item].x][currentCell.content.neighbourStatus(i).neighbours[item].y].content, CellType.WaterParasite);
                    break;
                  case CellType.SandParasite:
                    modifyCell(mapLayer.value[i + 1].positions[currentCell.content.neighbourStatus(i).neighbours[item].x][currentCell.content.neighbourStatus(i).neighbours[item].y].content, CellType.SandParasite);
                    break;
                  default:
                    break;
                }
              }
            }
          }
        }
      }
    }
  }
  isLoading.value = false;
}

onMounted(() => {
  let hrElement;
  let counter = 100;
  for (let i = 0; i < counter; i++) {
    hrElement = document.createElement('hr');
    hrElement.style.left = Math.floor(Math.random() * window.innerWidth) + 'px';
    hrElement.style.animationDuration = 0.2 + Math.random() * 0.3 + 's';
    hrElement.style.animationDelay = Math.random() * 5 + 's';
    const rainElement = document.getElementById('rain');
    if (rainElement !== null) {
      rainElement.appendChild(hrElement);
    } else {
      alert('rain element not found');
    }
  }
});
</script>

<template>
  <div id="rain" v-show="currentWeather === WeatherType.Rain"></div>
  <div id="sun" v-show="currentWeather === WeatherType.Sunlight">
    <div class="ray_box">
        <div class="ray ray1"></div>
        <div class="ray ray2"></div>
        <div class="ray ray3"></div>
        <div class="ray ray4"></div>
        <div class="ray ray5"></div>
        <div class="ray ray6"></div>
        <div class="ray ray7"></div>
        <div class="ray ray8"></div>
        <div class="ray ray9"></div>
        <div class="ray ray10"></div>
    </div>
  </div>
  <div class="h-screen overflow-hidden flex flex-col" :class="{'bg-blue-50': currentWeather === WeatherType.Normal, 'bg-gray-100': currentWeather === WeatherType.Rain, 'bg-yellow-100': currentWeather === WeatherType.Sunlight}">
    <div class="z-50 p-2 flex items-center shadow gap-3">
      <label>
        <span>行數</span>
        <InputNumber v-model="yAxis" inputClass="w-24" incrementButtonClass="p-0" decrementButtonclass="p-1" inputId="y-axis" mode="decimal" showButtons :min="0" :max="10000" :disabled="isPlaying" />
      </label>
      <label>
        <span>列數</span>
        <InputNumber v-model="xAxis" inputClass="w-24" incrementButtonClass="p-0" decrementButtonclass="p-1" inputId="x-axis" mode="decimal" showButtons :min="0" :max="10000" :disabled="isPlaying" />
      </label>
      <label>
        <span>迭代次數</span>
        <InputNumber v-model="iterations" inputClass="w-24" incrementButtonClass="p-0" decrementButtonclass="p-1" inputId="iterations" mode="decimal" showButtons :min="0" :max="10000" :disabled="isPlaying" />
      </label>
      <label>
        <span>間隔時間</span>
        <InputNumber v-model="duration" inputClass="w-24" incrementButtonClass="p-0" decrementButtonclass="p-1" inputId="duration" mode="decimal" showButtons :min="0" :max="10000" :disabled="isPlaying" />
      </label>
      <Button label="產生地圖" @click="initializeMap(randomMap, randomCell)" v-show="!isPlaying" />
      <label  v-show="!isPlaying">
        <span>隨機地圖</span>
        <input type="checkbox" v-model="randomMap">
      </label>
      <label v-show="!isPlaying">
        <span>隨機細胞</span>
        <input type="checkbox" v-model="randomCell">
      </label>
      <label v-show="isPlaying">迭代次數：{{ currentIteration + 1 }}</label>
      <Button label="計算" @click="CalculateEvolution" v-show="!isPlaying && mapLayer[currentIteration] != undefined && isLoading != true" />
      <Button label="開始" @click="startGame" v-show="!isPlaying && mapLayer[currentIteration] != undefined && isLoading != true" />
      <Button label="停止" @click="stopGame" v-show="isPlaying" />
    </div>
    <div class="flex-grow overflow-hidden h-full p-5">
      <div class="h-full overflow-auto">
        <table v-if="mapLayer[currentIteration] != undefined && isLoading != true" class="m-auto">
          <tr v-for="(row, index) in mapLayer[currentIteration].positions" :key="index">
            <template v-for="(item, index) in row" :key="index">
              <td :style="{'width': elementSize, 'height': elementSize}" :class="['border cursor-pointer leading-none', cellBackgroundHandler(item.type)]" @click="cellHandler(item)">
                <div :class="['w-full h-full rounded-full bg-opacity-80 p-auto text-center', cellColorHandler(item.content.type), item.content.type !== CellType.DeadCell ? 'shadow' : '']">
                  <div class="text-center inline-block" style="font-size: 8pt;" v-if="item.content.type === CellType.BigPredator || item.content.type === CellType.Predator">掠</div>
                  <div class="text-center inline-block" style="font-size: 8pt;" v-if="item.content.type === CellType.NormalParasite || item.content.type === CellType.WaterParasite || item.content.type === CellType.SandParasite">寄</div>
                </div>
              </td>
            </template>
          </tr>
        </table>
      </div>
    </div>
  </div>
  <Dialog v-model:visible="showModal" modal header="編輯本格內容" :style="{ width: '50rem' }" :breakpoints="{ '1199px': '75vw', '575px': '90vw' }">
    <template #footer>
      <Button label="取消" @click="showModal = false" />
      <Button label="儲存" icon="pi pi-check" @click="saveCellEdit" />
    </template>
    <div class="flex flex-col gap-5">
      <Dropdown v-model="selectedCell.type" :options="positionTypeList" optionLabel="label" placeholder="請選擇地形" optionValue="value" class="w-full md:w-14rem" />
      <Dropdown v-model="selectedCell.content.type" :options="cellTypeList" optionLabel="label" placeholder="請選擇細胞類型" optionValue="value" class="w-full md:w-14rem" />
      <InputNumber v-model="selectedCell.content.age" inputClass="w-full" incrementButtonClass="p-1" decrementButtonclass="p-1" inputId="age" mode="decimal" showButtons :min="0" :max="10000" class="w-full" />
    </div>
</Dialog>
</template>

<style lang="scss">
label {
  @apply flex items-center gap-2;
  span {
    @apply font-semibold text-xl;
  }
}
hr{
  width: 50px;
  border-color: transparent;
  border-right-color: black;
  border-right-width: 50px;
  position: absolute;
  bottom: 100%;
  transform-origin: 100% 50%;
  animation-name: rain;
  animation-duration: 1s;
  animation-timing-function: linear;
  animation-iteration-count: infinite;
}
@keyframes rain {
  from {
    transform: rotate(90deg) translateX(0);
  }
  to {
    transform: rotate(90deg) translateX(95vh);
  }
}
#sun { 
  position: absolute;
	top:0;
	left:0;
	right:0;
	bottom:0;
	margin: auto;  
	width:70px;
	height:70px;
	border-radius:50%;	
	background:white;
	opacity:0.8;			
	box-shadow: 0px 0px 40px 15px white;
  z-index: 10000;
}

.ray_box {
  position: absolute;
  margin: auto;
	top:0px;
	left:0;
	right:0;
	bottom:0;	
  width:70px;  
  -webkit-animation: ray_anim 120s linear infinite;
  animation: ray_anim 120s linear infinite;
}
.ray {  
    background: -webkit-linear-gradient(top, rgba(255,255,255,0) 0%, rgba(255,255,255,0.8) 50%, rgba(255,255,255,0) 100%);
		background: linear-gradient(top, rgba(255,255,255,0) 0%, rgba(255,255,255,0.8) 50%, rgba(255,255,255,0) 100%); 
    margin-left:10px;
    border-radius:80% 80% 0 0;
    position:absolute;
    opacity:0.1;
}

.ray1 {    
    height:170px;
    width:30px;
   -webkit-transform: rotate(180deg);
    top:-175px;
    left: 15px;
}
.ray2 {
    height:100px;
    width:8px;
   -webkit-transform: rotate(220deg);
    top:-90px;
    left: 75px;
}
.ray3 {
    height:170px;
    width:50px;
   -webkit-transform: rotate(250deg);
    top:-80px;
    left: 100px;
}
.ray4 {
    height:120px;
    width:14px;
   -webkit-transform: rotate(305deg);
    top:30px;
    left: 100px;
}
.ray5 {
    height:140px;
    width:30px;
   -webkit-transform: rotate(-15deg);
    top:60px;
    left: 40px;
}
.ray6 {
    height:90px;
    width:50px;
   -webkit-transform: rotate(30deg);
    top:60px;
    left: -40px;
}
.ray7 {
    height:180px;
    width:10px;
   -webkit-transform: rotate(70deg);
    top:-35px;
    left: -40px;
}
.ray8 {
    height:120px;
    width:30px;
   -webkit-transform: rotate(100deg);
    top:-45px;
    left:-90px;
}
.ray9 {
    height:80px;
    width:10px;
   -webkit-transform: rotate(120deg);
    top:-65px;
    left:-60px;
}
.ray10 {
    height:190px;
    width:23px;
   -webkit-transform: rotate(150deg);
    top:-185px;
    left: -60px;
}

@-webkit-keyframes ray_anim { 
		0% { -webkit-transform: rotate(0deg); transform: rotate(0deg);}    
    100% { -webkit-transform: rotate(360deg); transform: rotate(360deg);}
}
</style>
