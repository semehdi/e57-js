// tsup.config.js
export default {
  entry: ['src/js/index.js'],
  format: ['esm'],
  outDir: "./dist",
  dts: false,
  clean: true,

  async onSuccess() {
    await import('./scripts/post_build.js')
  }
}