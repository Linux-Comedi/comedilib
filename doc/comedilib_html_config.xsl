<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:fo="http://www.w3.org/1999/XSL/Format"
	version="1.0">
	<xsl:param name="use.id.as.filename" select="'1'"/>
	<xsl:param name="chunk.section.depth" select="2"/>
	<xsl:param name="generate.section.toc.level" select="4"/>
	<xsl:param name="html.stylesheet">comedilib.css</xsl:param>
	<xsl:param name="funcsynopsis.style">ansi</xsl:param>
	<xsl:param name="function.parens" select="1"/>
	<xsl:param name="section.autolabel" select="1"/>
	<xsl:param name="xref.with.number.and.title" select="0"/>
</xsl:stylesheet>
