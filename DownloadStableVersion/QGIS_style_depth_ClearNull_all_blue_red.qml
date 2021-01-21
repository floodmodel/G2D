<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis maxScale="0" styleCategories="AllStyleCategories" minScale="1e+08" hasScaleBasedVisibilityFlag="0" version="3.8.3-Zanzibar">
  <flags>
    <Identifiable>1</Identifiable>
    <Removable>1</Removable>
    <Searchable>1</Searchable>
  </flags>
  <customproperties>
    <property value="false" key="WMSBackgroundLayer"/>
    <property value="false" key="WMSPublishDataSourceUrl"/>
    <property value="0" key="embeddedWidgets/count"/>
    <property value="Value" key="identify/format"/>
  </customproperties>
  <pipe>
    <rasterrenderer classificationMin="1e-06" classificationMax="10" type="singlebandpseudocolor" opacity="1" band="1" alphaBand="-1">
      <rasterTransparency/>
      <minMaxOrigin>
        <limits>None</limits>
        <extent>WholeRaster</extent>
        <statAccuracy>Exact</statAccuracy>
        <cumulativeCutLower>0.02</cumulativeCutLower>
        <cumulativeCutUpper>0.98</cumulativeCutUpper>
        <stdDevFactor>2</stdDevFactor>
      </minMaxOrigin>
      <rastershader>
        <colorrampshader classificationMode="1" colorRampType="INTERPOLATED" clip="0">
          <colorramp name="[source]" type="gradient">
            <prop v="191,255,253,0" k="color1"/>
            <prop v="255,0,0,255" k="color2"/>
            <prop v="0" k="discrete"/>
            <prop v="gradient" k="rampType"/>
            <prop v="0.00480769;191,255,253,255:0.25;21,0,255,255" k="stops"/>
          </colorramp>
          <item color="#bffffd" label="0.000001" value="1e-06" alpha="0"/>
          <item color="#bffffd" label="0.01" value="0.01" alpha="255"/>
          <item color="#bcfafe" label="0.1" value="0.1" alpha="255"/>
          <item color="#b5f0fe" label="0.2" value="0.2" alpha="255"/>
          <item color="#a7dbfe" label="0.4" value="0.4" alpha="255"/>
          <item color="#7d9cfe" label="1.0" value="1" alpha="255"/>
          <item color="#5a68ff" label="1.5" value="1.5" alpha="255"/>
          <item color="#3734ff" label="2.0" value="2" alpha="255"/>
          <item color="#4400cc" label="4.0" value="4" alpha="255"/>
          <item color="#ff0000" label="10.0" value="10" alpha="255"/>
        </colorrampshader>
      </rastershader>
    </rasterrenderer>
    <brightnesscontrast contrast="0" brightness="0"/>
    <huesaturation colorizeStrength="100" grayscaleMode="0" colorizeOn="0" saturation="0" colorizeRed="255" colorizeGreen="128" colorizeBlue="128"/>
    <rasterresampler maxOversampling="2"/>
  </pipe>
  <blendMode>0</blendMode>
</qgis>
