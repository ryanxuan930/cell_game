<script setup lang="ts">
import { ref } from 'vue';

const width = ref(0);
const height = ref(0);
const iterations = ref(0);
const mapLayer = ref([]);

enum CellType {
  Normal,
  Water,
  Sand,
  Predator,
  
}

type TCell = {
  x: number;
  y: number;
  isAlive: boolean;
  type: CellType;
};
</script>

<template>
  <div></div>
</template>
