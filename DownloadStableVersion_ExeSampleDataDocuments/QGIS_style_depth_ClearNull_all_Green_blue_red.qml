<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>
<qgis maxScale="0" hasScaleBasedVisibilityFlag="0" version="3.8.3-Zanzibar" minScale="1e+08" styleCategories="AllStyleCategories">
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
    <rasterrenderer type="singlebandpseudocolor" band="1" classificationMin="1e-06" classificationMax="10" alphaBand="-1" opacity="1">
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
        <colorrampshader colorRampType="INTERPOLATED" clip="0" classificationMode="1">
          <colorramp type="gradient" name="[source]">
            <prop k="color1" v="3,255,19,0"/>
            <prop k="color2" v="255,0,0,255"/>
            <prop k="discrete" v="0"/>
            <prop k="rampType" v="gradient"/>
            <prop k="stops" v="0.00961538;74,255,56,255:0.216346;191,255,253,255:0.786058;0,0,255,255"/>
          </colorramp>
          <item value="1e-06" alpha="0" label="1e-06" color="#03ff13"/>
          <item value="0.0961548365384615" alpha="255" label="0.0961548365384615" color="#4aff38"/>
          <item value="2.16346232211539" alpha="255" label="2.16346232211539" color="#bffffd"/>
          <item value="7.86057713701923" alpha="255" label="7.86057713701923" color="#0000ff"/>
          <item value="10" alpha="255" label="10" color="#ff0000"/>
        </colorrampshader>
      </rastershader>
    </rasterrenderer>
    <brightnesscontrast brightness="0" contrast="0"/>
    <huesaturation colorizeOn="0" saturation="0" colorizeGreen="128" colorizeBlue="128" grayscaleMode="0" colorizeStrength="100" colorizeRed="255"/>
    <rasterresampler maxOversampling="2"/>
  </pipe>
  <blendMode>0</blendMode>
</qgis>
